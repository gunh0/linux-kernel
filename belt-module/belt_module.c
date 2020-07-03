#include <asm/byteorder.h>
#include <linux/init.h>
#include <linux/crypto.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/types.h>

#define BELT_KS				32
#define BELT_KS_DWORDS		8
#define BELT_BLOCK_SIZE		16
#define BELT_MIN_KEY_SIZE	32
#define BELT_MAX_KEY_SIZE	32

#define RotHi(x, r)			(((x) << (r)) | ((x) >> (32 - (r))))

#define U1(x)			( (x) >> 24 ) 
#define U2(x)   		(((x) >> 16 ) & 0xff )
#define U3(x)   		(((x) >> 8  ) & 0xff )
#define U4(x)   		( (x) & 0xff )

#define HU1(x,H)        	(((uint32_t) (H)[ U1((x)) ]) << 24)
#define HU2(x,H)        	(((uint32_t) (H)[ U2((x)) ]) << 16)
#define HU3(x,H)        	(((uint32_t) (H)[ U3((x)) ]) <<  8)
#define HU4(x,H)        	(((uint32_t) (H)[ U4((x)) ]))

#define G(x,H,r)        	RotHi(HU4((x),(H)) | HU3((x),(H)) | HU2((x),(H)) | HU1((x),(H)),(r))
#define SWAP(a,b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

struct belt_ctx {
	u32 ks[BELT_KS_DWORDS];
};

const u8 H[256] =
{
        0xB1, 0x94, 0xBA, 0xC8, 0x0A, 0x08, 0xF5, 0x3B, 0x36, 0x6D, 0x00, 0x8E, 0x58, 0x4A, 0x5D, 0xE4,
        0x85, 0x04, 0xFA, 0x9D, 0x1B, 0xB6, 0xC7, 0xAC, 0x25, 0x2E, 0x72, 0xC2, 0x02, 0xFD, 0xCE, 0x0D,
        0x5B, 0xE3, 0xD6, 0x12, 0x17, 0xB9, 0x61, 0x81, 0xFE, 0x67, 0x86, 0xAD, 0x71, 0x6B, 0x89, 0x0B,
        0x5C, 0xB0, 0xC0, 0xFF, 0x33, 0xC3, 0x56, 0xB8, 0x35, 0xC4, 0x05, 0xAE, 0xD8, 0xE0, 0x7F, 0x99,
        0xE1, 0x2B, 0xDC, 0x1A, 0xE2, 0x82, 0x57, 0xEC, 0x70, 0x3F, 0xCC, 0xF0, 0x95, 0xEE, 0x8D, 0xF1,
        0xC1, 0xAB, 0x76, 0x38, 0x9F, 0xE6, 0x78, 0xCA, 0xF7, 0xC6, 0xF8, 0x60, 0xD5, 0xBB, 0x9C, 0x4F,
        0xF3, 0x3C, 0x65, 0x7B, 0x63, 0x7C, 0x30, 0x6A, 0xDD, 0x4E, 0xA7, 0x79, 0x9E, 0xB2, 0x3D, 0x31,
        0x3E, 0x98, 0xB5, 0x6E, 0x27, 0xD3, 0xBC, 0xCF, 0x59, 0x1E, 0x18, 0x1F, 0x4C, 0x5A, 0xB7, 0x93,
        0xE9, 0xDE, 0xE7, 0x2C, 0x8F, 0x0C, 0x0F, 0xA6, 0x2D, 0xDB, 0x49, 0xF4, 0x6F, 0x73, 0x96, 0x47,
        0x06, 0x07, 0x53, 0x16, 0xED, 0x24, 0x7A, 0x37, 0x39, 0xCB, 0xA3, 0x83, 0x03, 0xA9, 0x8B, 0xF6,
        0x92, 0xBD, 0x9B, 0x1C, 0xE5, 0xD1, 0x41, 0x01, 0x54, 0x45, 0xFB, 0xC9, 0x5E, 0x4D, 0x0E, 0xF2,
        0x68, 0x20, 0x80, 0xAA, 0x22, 0x7D, 0x64, 0x2F, 0x26, 0x87, 0xF9, 0x34, 0x90, 0x40, 0x55, 0x11,
        0xBE, 0x32, 0x97, 0x13, 0x43, 0xFC, 0x9A, 0x48, 0xA0, 0x2A, 0x88, 0x5F, 0x19, 0x4B, 0x09, 0xA1,
        0x7E, 0xCD, 0xA4, 0xD0, 0x15, 0x44, 0xAF, 0x8C, 0xA5, 0x84, 0x50, 0xBF, 0x66, 0xD2, 0xE8, 0x8A,
        0xA2, 0xD7, 0x46, 0x52, 0x42, 0xA8, 0xDF, 0xB3, 0x69, 0x74, 0xC5, 0x51, 0xEB, 0x23, 0x29, 0x21,
        0xD4, 0xEF, 0xD9, 0xB4, 0x3A, 0x62, 0x28, 0x75, 0x91, 0x14, 0x10, 0xEA, 0x77, 0x6C, 0xDA, 0x1D
};

const u32 KeyIndex[8][7] =
{
        { 0, 1, 2, 3, 4, 5, 6 },
        { 7, 0, 1, 2, 3, 4, 5 },
        { 6, 7, 0, 1, 2, 3, 4 },
        { 5, 6, 7, 0, 1, 2, 3 },
        { 4, 5, 6, 7, 0, 1, 2 },
        { 3, 4, 5, 6, 7, 0, 1 },
        { 2, 3, 4, 5, 6, 7, 0 },
        { 1, 2, 3, 4, 5, 6, 7 }
};


/* Only 256-bit keys are used and implemented */
int belt_setkey(struct crypto_tfm *tfm, const u8 *key, unsigned int key_len)
{
	struct belt_ctx *ctx = crypto_tfm_ctx(tfm);
	u32 *flags = &tfm->crt_flags;	
	const __le32 *k = (const __le32 *)key;
	int i;

	if(key_len != BELT_MIN_KEY_SIZE)
	{
		*flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
		return -EINVAL;
	}

	for(i = 0; i < BELT_KS_DWORDS; i++)
		ctx->ks[i] = le32_to_cpu(k[i]);

	return 0;	
}

/* Encrypt one block. In and out may be the same. */
static void belt_encrypt(struct crypto_tfm *tfm, u8 *out, const u8 *in)
{
	const struct belt_ctx *ctx = crypto_tfm_ctx(tfm);
	const __le32 *src = (const __le32 *)in;
	const u32 *key = (const u32 *)ctx->ks;
	__le32 *dst = (__le32 *)out;
	u32 a[4], e, i;

	a[0] = le32_to_cpu(src[0]);
	a[1] = le32_to_cpu(src[1]);
	a[2] = le32_to_cpu(src[2]);
	a[3] = le32_to_cpu(src[3]);

	for(i = 0; i < 8; ++i)
	{
		a[1] ^= G((a[0] + key[KeyIndex[i][0]]), H, 5);
                a[2] ^= G((a[3] + key[KeyIndex[i][1]]), H, 21);
                a[0] = a[0] - G(a[1] + key[KeyIndex[i][2]], H, 13);
                e = G((a[1] + a[2] + key[KeyIndex[i][3]]), H, 21) ^ (i + 1);
                a[1] += e;
                a[2] = a[2] - e;
                a[3] += G(a[2] + key[KeyIndex[i][4]], H, 13);
                a[1] ^= G(a[0] + key[KeyIndex[i][5]], H, 21);
                a[2] ^= G(a[3] + key[KeyIndex[i][6]], H, 5);
                SWAP(a[0], a[1]);
                SWAP(a[2], a[3]);
                SWAP(a[1], a[2]);
	}

	dst[0] = cpu_to_le32(a[1]);
	dst[1] = cpu_to_le32(a[3]);
	dst[2] = cpu_to_le32(a[0]);
	dst[3] = cpu_to_le32(a[2]);
}

/* Decrypt one block.  in and out may be the same. */
static void belt_decrypt(struct crypto_tfm *tfm, u8 *out, const u8 *in)
{
	const struct belt_ctx *ctx = crypto_tfm_ctx(tfm);
	const __le32 *src = (const __le32 *)in;
	__le32 *dst = ( __le32 *)out;
	const u32 *key = (const u32 *)ctx->ks; 	
	u32 a[4], e;
	int i;

	a[0] = le32_to_cpu(src[0]);
	a[1] = le32_to_cpu(src[1]);
	a[2] = le32_to_cpu(src[2]);
	a[3] = le32_to_cpu(src[3]);

	for(i = 7; i >= 0; i--)
	{
		a[1] ^= G(a[0] + key[KeyIndex[i][6]], H, 5);
                a[2] ^= G(a[3] + key[KeyIndex[i][5]], H, 21);
                a[0] = a[0] - G(a[1] + key[KeyIndex[i][4]], H, 13);
                e = G(a[1] + a[2] + key[KeyIndex[i][3]], H, 21) ^ (i + 1);
                a[1] += e;
                a[2] = a[2] - e;
                a[3] += G(a[2] + key[KeyIndex[i][2]], H, 13);
                a[1] ^= G(a[0] + key[KeyIndex[i][1]], H, 21);
                a[2] ^= G(a[3] + key[KeyIndex[i][0]], H, 5);
                SWAP(a[0], a[1]);
                SWAP(a[2], a[3]);
                SWAP(a[0], a[3]);
	}	

	dst[0] = cpu_to_le32(a[2]);
	dst[1] = cpu_to_le32(a[0]);
	dst[2] = cpu_to_le32(a[3]);
	dst[3] = cpu_to_le32(a[1]);	
}

static struct crypto_alg alg = {
	.cra_name		=	"belt",
	.cra_driver_name	=	"belt-generic",
	.cra_priority		=	100,
	.cra_flags		=	CRYPTO_ALG_TYPE_CIPHER,
	.cra_blocksize		=	BELT_BLOCK_SIZE,
	.cra_ctxsize		=	sizeof(struct belt_ctx),
	.cra_alignmask		=	3,
	.cra_module		=	THIS_MODULE,
	.cra_list		=	LIST_HEAD_INIT(alg.cra_list),
	.cra_u			=	{ 
		.cipher = {
			.cia_min_keysize	=	BELT_MIN_KEY_SIZE,
			.cia_max_keysize	=	BELT_MAX_KEY_SIZE,
			.cia_setkey		=	belt_setkey,
			.cia_encrypt		=	belt_encrypt,
			.cia_decrypt		=	belt_decrypt 
		} 
	}
};

static int __init belt_mod_init(void)
{
	printk("Belt module loaded\n");
	return crypto_register_alg(&alg);
}

static void __exit belt_mod_fini(void)
{
	printk("Belt module unloaded\n");
	crypto_unregister_alg(&alg);
}

module_init(belt_mod_init);
module_exit(belt_mod_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Belt block cipher (256 bit key)");