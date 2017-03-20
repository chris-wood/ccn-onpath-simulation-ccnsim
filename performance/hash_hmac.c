#include <sodium.h>
#include <stdio.h>

#include "timer.h"

int
main(int argc, char **argv)
{
    if (sodium_init() == -1) {
        printf("ah!\n");
        return -1;
    }

    if (argc != 3) {
        return -1;
    }

    int msgLength = atoi(argv[1]);
    int k = atoi(argv[2]);

    unsigned char hash[crypto_generichash_BYTES];
    unsigned char key[crypto_auth_KEYBYTES];
    unsigned char mac[crypto_auth_BYTES];

    char *message = (char *) malloc(msgLength);
    for (int i = 0; i < msgLength; i++) {
        message[i] = (char) i;
    }


    // Compute the HMAC
    struct timespec start = timerStart();
    crypto_generichash(hash, sizeof(hash), message, msgLength, NULL, 0);
    for (int i = 0; i < k; i++) {
        crypto_auth(mac, hash, sizeof(hash), key);
    }
    long duration = timerEnd(start);
    printf("%d\n", duration);

    free(message);

    return 0;
}
