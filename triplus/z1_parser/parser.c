/* -----------------------------------------------------------------------------
 * parser_template.c  —  Kostur parsera za Zadatak 1
 *
 * Dopunite označene TODO sekcije. NE mijenjajte javni API (deklaracije funkcija
 * i signature callback-a) — koristi ih evaluator za automatsko testiranje.
 *
 * Kompajlirati sa:  gcc -std=c11 -Wall -Wextra parser_template.c -o parser
 * -------------------------------------------------------------------------- */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
/* Trebamo memchr — u nekim MCU toolchainovima nije <string.h>. */																	
#include <string.h>

#define BUF_SIZE	256

/* ------------------- JAVNI API (ne mijenjati) -------------------------------*/

/* Statusi koje callback prima. */								  
typedef enum {
	FRAME_OK          = 0,
	FRAME_BAD_CRC     = 1,
	FRAME_BAD_LENGTH  = 2,
	FRAME_BAD_ESCAPE  = 3,
	FRAME_TRUNCATED   = 4,
} frame_status_t;

/* Callback koji se poziva za SVAKI kompletan pokušaj okvira (valjan ili ne).
 *   status  -- FRAME_OK ili razlog odbacivanja
 *   cmd_code     -- cmd_code bajt (validan samo ako status == FRAME_OK)
 *   payload -- pointer na DEKODIRAN payload (nakon de-escape); NULL ako !OK
 *   length  -- broj bajtova u payload-u
 *   user    -- neprozirni pointer koji je predan parser_reset()
 */
typedef void (*frame_cb_t)(frame_status_t status,
						   uint8_t cmd_code,
						   const uint8_t *payload,
						   size_t length,
						   void *user);

/* Inicijalizacija/reset parsera. Poziva se jednom prije prvog byte-a. */
void parser_reset(frame_cb_t cb, void *user);

/* Feed-a jedan bajt. Zovi za svaki bajt s žice, redom kojim su stigli.
 * Parser interno održava stanje. Kad detektira kraj okvira (valjan ili ne),
 * poziva callback. */
 void parser_feed(uint8_t b);

/* Pomoćna funkcija — implementirajte je i koristite iz svog parsera. */
uint8_t crc8_maxim(const uint8_t *data, size_t len);

typedef enum {
	STATE_1,
	STATE_2,
	STATE_3
} parser_state_t;

static frame_cb_t cb_f = NULL;
static void *user_f = NULL;

static parser_state_t state = STATE_1;
static uint8_t first_preamble = 0;

static uint8_t buf[BUF_SIZE];
static size_t buf_index = 0;
static size_t expected_len = 0;
static bool escape = false;

void parser_reset(frame_cb_t cb, void *user) {
	cb_f = cb;
	user_f = user;
	state = STATE_1;
	first_preamble = 0;
	buf_index = 0;
	expected_len = 0;
	escape = false;
}

uint8_t crc8_maxim(const uint8_t *data, size_t len) {
	uint8_t crc = 0x00;

	for (size_t i = 0; i < len; i++) {
		crc ^= data[i];
		for (int j = 0; j < 8; j++) {
			if (crc & 0x01) {
				crc = (crc >> 1) ^ 0xE5; /* Poly: 0xA7,  RefIn i RefOut = true, poly bit reversed - 0xE5, Init = 0x00, XorOut = 0x00  */
			} else {
				crc >>= 1;
			}
		}
	}
	return crc;
}

void parser_feed(uint8_t b) {
	switch (state) {
		case STATE_1:
			if (b == 0xAA || b == 0x55) {
				first_preamble = b;
				state = STATE_2;
			}
			break;

		case STATE_2:
			if ((first_preamble == 0xAA && b == 0x55) ||
				(first_preamble == 0x55 && b == 0xAA)) {
				state = STATE_3;
				buf_index = 0;
				expected_len = 0;
				escape = false;
			} else {
				state = STATE_1;
			}
			break;

		case STATE_3:
			if (!escape && (b == 0xAA || b == 0x55)) {
				buf_index = 0;
				expected_len = 0;
				escape = false;
				first_preamble = b;
				state = STATE_2;
				cb_f(FRAME_TRUNCATED, 0, NULL, 0, user_f);
				break;
			}

			if (escape) {
				escape = false;
				uint8_t orig_frame = b ^ 0x20;
				if (buf_index < sizeof(buf)) {
					buf[buf_index++] = orig_frame;
				}
			} else {
				if (b == 0x7D) {
					escape = true;
					break;
				}
				if (buf_index < sizeof(buf)) {
					buf[buf_index++] = b;
				}
			}

			if (buf_index == 1) {
				uint8_t len = buf[0];
				if (len < 1 ) {
					cb_f(FRAME_BAD_LENGTH, 0, NULL, 0, user_f);
					state = STATE_1;
					break;
				}
				expected_len = (size_t)len + 2; /* (2 bajta za CRC i za LEN) */
			}

			if (buf_index >= sizeof(buf)) {
				cb_f(FRAME_BAD_LENGTH, 0, NULL, 0, user_f);
				state = STATE_1;
				break;
			}

			if (expected_len > 0 && buf_index == expected_len) {
				uint8_t len = buf[0];
				uint8_t cmd_code = buf[1];
				const uint8_t *payload = &buf[2];
				size_t payload_len = len - 1;
				uint8_t rx_crc = buf[expected_len - 1];
				uint8_t calc_crc = crc8_maxim(buf, expected_len - 1);

				if (calc_crc == rx_crc) {
					cb_f(FRAME_OK, cmd_code, payload, payload_len, user_f);
				} else {
					cb_f(FRAME_BAD_CRC, 0, NULL, 0, user_f);
				}
				state = STATE_1;
			}
			break;
	}
}

/* ------------------- MAIN (ne mijenjati bitno) ----------------------------- */

static void print_cb(frame_status_t st, uint8_t cmd_code, const uint8_t *p,
					 size_t n, void *user) {
	(void)user;
	if (st != FRAME_OK) {
		printf("[BAD ] status=%d\n", st);
		return;
	}
	printf("[ OK ] cmd_code=0x%02X len=%zu payload=", cmd_code, n);
	for (size_t i = 0; i < n; i++) printf("%02X ", p[i]);
	printf("\n");
}

int main(int argc, char **argv) {
	if (argc < 2) { fprintf(stderr, "Usage: %s capture.txt\n", argv[0]); return 1; }
	FILE *f = fopen(argv[1], "r");
	if (!f) { perror("fopen"); return 1; }
	parser_reset(print_cb, NULL);

	char line[1024];
	while (fgets(line, sizeof line, f)) {
		if (line[0] == '#' || line[0] == '\n') continue;
		/* Očekujemo redak oblika "t=... MOSI: 0xAA 0x55 ..." ili slično.
		 * Ekstraktiraj sve "0xXX" tokene i feedaj ih. */
		char *p = line;
		while ((p = (char *)memchr(p, '0', (line + sizeof line) - p))) {
			if (p[1] == 'x' || p[1] == 'X') {
				unsigned v; if (sscanf(p, "0x%2x", &v) == 1) parser_feed((uint8_t)v);
				p += 4;
			} else {
				p++;
			}
		}
	}
	fclose(f);
	return 0;
}