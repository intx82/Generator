/* aes.c */

#include <stdint.h>
#include <string.h>
#include "aes.h"

/**< https://github.com/DavyLandman/AESLib */

typedef struct
{
    uint8_t ks[16];
} roundkey_t;

typedef struct
{
    uint8_t s[16];
} cipher_state_t;

typedef struct
{
    roundkey_t key[10 + 1];
} aes128_ctx_t;

const uint8_t sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};

const uint8_t invsbox[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d};

static uint8_t gf256mul(uint8_t a, uint8_t b, uint8_t reducer)
{
    uint8_t i;
    uint8_t p = 0, t;
    for (i = 0; i < 8; ++i)
    {
        if (b & 1)
        {
            p ^= a;
        }
        t = a & 0x80;
        a <<= 1;
        if (t)
        {
            a ^= reducer;
        }
        b >>= 1;
    }
    return p;
}

static void rotword(void *a)
{
    uint8_t t;
    t = ((uint8_t *)a)[0];
    ((uint8_t *)a)[0] = ((uint8_t *)a)[1];
    ((uint8_t *)a)[1] = ((uint8_t *)a)[2];
    ((uint8_t *)a)[2] = ((uint8_t *)a)[3];
    ((uint8_t *)a)[3] = t;
}

const uint8_t rc_tab[] = {0x01, 0x02, 0x04, 0x08,
                          0x10, 0x20, 0x40, 0x80,
                          0x1b, 0x36};

static void init(void *key, uint16_t keysize_b, aes128_ctx_t *ctx)
{
    uint8_t hi, i, nk, next_nk;
    uint8_t rc = 0;
    union
    {
        uint32_t v32;
        uint8_t v8[4];
    } tmp;
    nk = keysize_b >> 5; /* 4, 6, 8 */
    hi = 4 * (nk + 6 + 1);
    memcpy(ctx, key, keysize_b / 8);
    next_nk = nk;
    for (i = nk; i < hi; ++i)
    {
        tmp.v32 = ((uint32_t *)(ctx->key[0].ks))[i - 1];
        if (i != next_nk)
        {
            if (nk == 8 && i % 8 == 4)
            {
                tmp.v8[0] = sbox[(tmp.v8[0])];
                tmp.v8[1] = sbox[(tmp.v8[1])];
                tmp.v8[2] = sbox[(tmp.v8[2])];
                tmp.v8[3] = sbox[(tmp.v8[3])];
            }
        }
        else
        {
            next_nk += nk;
            rotword(&(tmp.v32));
            tmp.v8[0] = sbox[(tmp.v8[0])];
            tmp.v8[1] = sbox[(tmp.v8[1])];
            tmp.v8[2] = sbox[(tmp.v8[2])];
            tmp.v8[3] = sbox[(tmp.v8[3])];
            tmp.v8[0] ^= rc_tab[rc];
            rc++;
        }
        ((uint32_t *)(ctx->key[0].ks))[i] = ((uint32_t *)(ctx->key[0].ks))[i - nk] ^ tmp.v32;
    }
}

static void shiftcol(void *data, uint8_t shift)
{
    uint8_t tmp[4];
    tmp[0] = ((uint8_t *)data)[0];
    tmp[1] = ((uint8_t *)data)[4];
    tmp[2] = ((uint8_t *)data)[8];
    tmp[3] = ((uint8_t *)data)[12];
    ((uint8_t *)data)[0] = tmp[(shift + 0) & 3];
    ((uint8_t *)data)[4] = tmp[(shift + 1) & 3];
    ((uint8_t *)data)[8] = tmp[(shift + 2) & 3];
    ((uint8_t *)data)[12] = tmp[(shift + 3) & 3];
}

#define GF256MUL_1(a) (a)
#define GF256MUL_2(a) (gf256mul(2, (a), 0x1b))
#define GF256MUL_3(a) (gf256mul(3, (a), 0x1b))

static void enc_round(cipher_state_t *state, const roundkey_t *k)
{
    uint8_t tmp[16], t;
    uint8_t i;
    /* subBytes */
    for (i = 0; i < 16; ++i)
    {
        tmp[i] = sbox[state->s[i]];
    }
    /* shiftRows */
    shiftcol(tmp + 1, 1);
    shiftcol(tmp + 2, 2);
    shiftcol(tmp + 3, 3);
    /* mixColums */
    for (i = 0; i < 4; ++i)
    {
        t = tmp[4 * i + 0] ^ tmp[4 * i + 1] ^ tmp[4 * i + 2] ^ tmp[4 * i + 3];
        state->s[4 * i + 0] =
            GF256MUL_2(tmp[4 * i + 0] ^ tmp[4 * i + 1]) ^ tmp[4 * i + 0] ^ t;
        state->s[4 * i + 1] =
            GF256MUL_2(tmp[4 * i + 1] ^ tmp[4 * i + 2]) ^ tmp[4 * i + 1] ^ t;
        state->s[4 * i + 2] =
            GF256MUL_2(tmp[4 * i + 2] ^ tmp[4 * i + 3]) ^ tmp[4 * i + 2] ^ t;
        state->s[4 * i + 3] =
            GF256MUL_2(tmp[4 * i + 3] ^ tmp[4 * i + 0]) ^ tmp[4 * i + 3] ^ t;
    }

    /* addKey */
    for (i = 0; i < 16; ++i)
    {
        state->s[i] ^= k->ks[i];
    }
}

static void enc_lastround(cipher_state_t *state, const roundkey_t *k)
{
    uint8_t i;
    /* subBytes */
    for (i = 0; i < 16; ++i)
    {
        state->s[i] = sbox[state->s[i]];
    }
    /* shiftRows */
    shiftcol(state->s + 1, 1);
    shiftcol(state->s + 2, 2);
    shiftcol(state->s + 3, 3);
    /* keyAdd */
    for (i = 0; i < 16; ++i)
    {
        state->s[i] ^= k->ks[i];
    }
}

static void encrypt_core(cipher_state_t *state, const aes128_ctx_t *ks, uint8_t rounds)
{
    uint8_t i;
    for (i = 0; i < 16; ++i)
    {
        state->s[i] ^= ks->key[0].ks[i];
    }
    i = 1;
    for (; rounds > 1; --rounds)
    {
        enc_round(state, &(ks->key[i]));
        ++i;
    }
    enc_lastround(state, &(ks->key[i]));
}

static void invshiftrow(void *data, uint8_t shift)
{
    uint8_t tmp[4];
    tmp[0] = ((uint8_t *)data)[(4 + 0 - shift) & 3];
    tmp[1] = ((uint8_t *)data)[(4 + 1 - shift) & 3];
    tmp[2] = ((uint8_t *)data)[(4 + 2 - shift) & 3];
    tmp[3] = ((uint8_t *)data)[(4 + 3 - shift) & 3];
    memcpy(data, tmp, 4);
}

static void invshiftcol(void *data, uint8_t shift)
{
    uint8_t tmp[4];
    tmp[0] = ((uint8_t *)data)[0];
    tmp[1] = ((uint8_t *)data)[4];
    tmp[2] = ((uint8_t *)data)[8];
    tmp[3] = ((uint8_t *)data)[12];
    ((uint8_t *)data)[0] = tmp[(4 - shift + 0) & 3];
    ((uint8_t *)data)[4] = tmp[(4 - shift + 1) & 3];
    ((uint8_t *)data)[8] = tmp[(4 - shift + 2) & 3];
    ((uint8_t *)data)[12] = tmp[(4 - shift + 3) & 3];
}

static void dec_round(cipher_state_t *state, const roundkey_t *k)
{
    uint8_t tmp[16];
    uint8_t i;
    uint8_t t, u, v, w;
    /* keyAdd */
    for (i = 0; i < 16; ++i)
    {
        tmp[i] = state->s[i] ^ k->ks[i];
    }
    /* mixColums */
    for (i = 0; i < 4; ++i)
    {
        t = tmp[4 * i + 3] ^ tmp[4 * i + 2];
        u = tmp[4 * i + 1] ^ tmp[4 * i + 0];
        v = t ^ u;
        v = gf256mul(0x09, v, 0x1b);
        w = v ^ gf256mul(0x04, tmp[4 * i + 2] ^ tmp[4 * i + 0], 0x1b);
        v = v ^ gf256mul(0x04, tmp[4 * i + 3] ^ tmp[4 * i + 1], 0x1b);
        state->s[4 * i + 3] = tmp[4 * i + 3] ^ v ^ gf256mul(0x02, tmp[4 * i + 0] ^ tmp[4 * i + 3], 0x1b);
        state->s[4 * i + 2] = tmp[4 * i + 2] ^ w ^ gf256mul(0x02, t, 0x1b);
        state->s[4 * i + 1] = tmp[4 * i + 1] ^ v ^ gf256mul(0x02, tmp[4 * i + 2] ^ tmp[4 * i + 1], 0x1b);
        state->s[4 * i + 0] = tmp[4 * i + 0] ^ w ^ gf256mul(0x02, u, 0x1b);

        /*
        state->s[4*i+0] =
        	  gf256mul(0xe, tmp[4*i+0], 0x1b)
        	^ gf256mul(0xb, tmp[4*i+1], 0x1b)
        	^ gf256mul(0xd, tmp[4*i+2], 0x1b)
        	^ gf256mul(0x9, tmp[4*i+3], 0x1b);
        state->s[4*i+1] =
        	  gf256mul(0x9, tmp[4*i+0], 0x1b)
        	^ gf256mul(0xe, tmp[4*i+1], 0x1b)
        	^ gf256mul(0xb, tmp[4*i+2], 0x1b)
        	^ gf256mul(0xd, tmp[4*i+3], 0x1b);
        state->s[4*i+2] =
        	  gf256mul(0xd, tmp[4*i+0], 0x1b)
        	^ gf256mul(0x9, tmp[4*i+1], 0x1b)
        	^ gf256mul(0xe, tmp[4*i+2], 0x1b)
        	^ gf256mul(0xb, tmp[4*i+3], 0x1b);
        state->s[4*i+3] =
        	  gf256mul(0xb, tmp[4*i+0], 0x1b)
        	^ gf256mul(0xd, tmp[4*i+1], 0x1b)
        	^ gf256mul(0x9, tmp[4*i+2], 0x1b)
        	^ gf256mul(0xe, tmp[4*i+3], 0x1b);
        */
    }
    /* shiftRows */
    invshiftcol(state->s + 1, 1);
    invshiftcol(state->s + 2, 2);
    invshiftcol(state->s + 3, 3);
    /* subBytes */
    for (i = 0; i < 16; ++i)
    {
        state->s[i] = invsbox[state->s[i]];
    }
}

static void dec_firstround(cipher_state_t *state, const roundkey_t *k)
{
    uint8_t i;
    /* keyAdd */
    for (i = 0; i < 16; ++i)
    {
        state->s[i] ^= k->ks[i];
    }
    /* shiftRows */
    invshiftcol(state->s + 1, 1);
    invshiftcol(state->s + 2, 2);
    invshiftcol(state->s + 3, 3);
    /* subBytes */
    for (i = 0; i < 16; ++i)
    {
        state->s[i] = invsbox[state->s[i]];
    }
}
static void _xor(uint8_t *a, uint8_t *b, uint8_t a_len, uint8_t b_len)
{
    uint8_t len = b_len;
    if (a <= b)
    {
        len = a_len;
    }
    for (uint8_t i = 0; i < len; i++)
    {
        a[i] ^= b[i];
    }
}

static void decrypt_core(cipher_state_t *state, const aes128_ctx_t *ks, uint8_t rounds)
{
    uint8_t i;
    dec_firstround(state, &(ks->key[i = rounds]));
    for (; rounds > 1; --rounds)
    {
        --i;
        dec_round(state, &(ks->key[i]));
    }
    for (i = 0; i < 16; ++i)
    {
        state->s[i] ^= ks->key[0].ks[i];
    }
}

void aes128_dec(uint8_t *key, uint8_t *data, mode_t mode, uint8_t *iv)
{
    aes128_ctx_t ctx = {};
    init(key, 128, (aes128_ctx_t *)&ctx);
    switch (mode)
    {
    case AES_ECB:
    {
        decrypt_core((cipher_state_t *)data, (aes128_ctx_t *)&ctx, 10);
        return;
    };
    case AES_CBC:
    {
        if (!iv)
        {
            return;
        }
        uint16_t tmp_data[16] = {};
        memcpy(tmp_data, data, 16);
        //aes_decrypt_core((cipher_state_t*)data, (aes128_ctx_t*)&ctx, 10);
        decrypt_core((cipher_state_t *)data, (aes128_ctx_t *)&ctx, 10);
        _xor(data, iv, 16, 16);
        memcpy(iv, tmp_data, 16);
        return;
    }
    default:
        return;
    }
}

void aes128_enc(uint8_t *key, uint8_t *data, mode_t mode, uint8_t *iv)
{
    aes128_ctx_t ctx = {};
    init(key, 128, (aes128_ctx_t *)&ctx);
    switch (mode)
    {
    case AES_ECB:
    {
        encrypt_core((cipher_state_t *)data, (aes128_ctx_t *)&ctx, 10);
        return;
    };
    case AES_CBC:
    {
        if (!iv)
        {
            return;
        }
 
        _xor(data, iv, 16, 16);
        encrypt_core((cipher_state_t *)data, (aes128_ctx_t *)&ctx, 10);
        memcpy(iv, data, 16);
        return;
    }
    default:
        return;
    }
}

