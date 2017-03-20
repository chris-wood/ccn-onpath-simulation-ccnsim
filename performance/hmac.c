#include <sodium.h>

int
main(int argc, char **argv)
{
    if (sodium_init() == -1) {
        return -1;
    }

    int msgLength = 1024;
    if (argc == 2) {
        msgLength = atoi(argv[1]);
    }

    unsigned char key[crypto_auth_KEYBYTES];
    unsigned char mac[crypto_auth_BYTES];

    char *message = (char *) malloc(msgLength);
    for (int i = 0; i < msgLength; i++) {
        message[i] = (char) i;
    }

    // Compute the HMAC
    crypto_auth(mac, message, msgLength, key);

    free(message);

    return 0;
}
