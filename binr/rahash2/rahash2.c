/* radare - LGPL - Copyright 2009-2011 pancake<nopcode.org> */

#include <stdio.h>
#include <string.h>
#include <getopt.h>
/* r2 api */
#include <r_io.h>
#include <r_hash.h>
#include <r_util.h>
#include <r_print.h>

static int do_hash_internal(RHash *ctx, ut64 from, int hash, const ut8 *buf, int len, int rad) {
	const ut8 *c = ctx->digest;
	int i, dlen;
	const char *hname = r_hash_name (hash);
	dlen = r_hash_calculate (ctx, hash, buf, len);
	if (!dlen || rad == 2)
		return 0;
	if (rad) {
		printf ("e file.%s=", hname);
		for (i=0; i<dlen; i++)
			printf ("%02x", c[i]);
		printf ("\n");
	} else {
		if (hash == R_HASH_ENTROPY) {
			double e = r_hash_entropy (buf, len);
			printf ("0x%08"PFMT64x"-0x%08"PFMT64x" %10f: ", 
				from, from+len, e);
			r_print_progressbar (NULL, 12.5 * e, 60);
			printf ("\n");
		} else {
			printf ("0x%08"PFMT64x"-0x%08"PFMT64x" %s: ", 
				from, from+len, hname);
			for (i=0; i<dlen; i++)
				printf ("%02x", c[i]);
			printf ("\n");
		}
	}
	return 1;
}

static int do_hash(const char *algo, RIO *io, int bsize, int rad) {
	ut8 *buf;
	RHash *ctx;
	ut64 j, fsize;
	int i;
	ut64 algobit = r_hash_name_to_bits (algo);
	if (algobit == R_HASH_NONE) {
		eprintf ("Invalid hashing algorithm specified\n");
		return 1;
	}
	fsize = r_io_size (io);
	if (bsize == 0 || bsize > fsize)
		bsize = fsize;
	if (fsize == -1LL) {
		eprintf ("Unknown file size\n");
		return 1;
	}
	buf = malloc (bsize+1);
	ctx = r_hash_new (R_TRUE, algobit);
	/* iterate over all algorithm bits */
	for (i=1; i<0x800000; i<<=1) {
		if (algobit & i) {
			for (j=0; j<fsize; j+=bsize) {
				r_io_read_at (io, j, buf, bsize);
				if (j+bsize<fsize)
					do_hash_internal (ctx, j, i, buf, bsize, rad);
				else do_hash_internal (ctx, j, i, buf, fsize-j, rad); // finish him!
			}
		}
	} 
	r_hash_free (ctx);
	free (buf);
	return 0;
}

static int do_help(int line) {
	printf ("Usage: rahash2 [-rV] [-b bsize] [-a algo] [-s str] [file] ...\n");
	if (line) return 0;
	printf (
	" -a algo     comma separated list of algorithms (default is 'sha1')\n"
	" -b bsize    specify the size of the block (instead of full file)\n"
	" -s string   hash this string instead of files\n"
	" -r          output radare commands\n"
	" -V          show version information\n"
	"Supported algorithms: md4, md5, sha1, sha256, sha384, sha512, crc16,\n"
	"    crc32, xor, xorpair, parity, mod255, hamdist, entropy, pcprint\n");
	return 0;
}

int main(int argc, char **argv) {
	RIO *io;
	const char *algo = "md5,sha1"; /* default hashing algorithm */
	const ut8 *buf = NULL;
	int c, buf_len = 0;
	int bsize = 0;
	int rad = 0;

	while ((c = getopt (argc, argv, "rVa:s:b:h")) != -1) {
		switch (c) {
		case 'r':
			rad = 1;
			break;
		case 'a':
			algo = optarg;
			break;
		case 'b':
			bsize = (int)r_num_math (NULL, optarg);
			break;
		case 's':
			buf = (const ut8*) optarg;
			buf_len = strlen (optarg);
			break;
		case 'V':
			printf ("rahash2 v"R2_VERSION"\n");
			return 0;
		case 'h':
			return do_help (0);
		}
	}

	if (optind>=argc)
		return do_help (1);

	io = r_io_new ();
	if (!r_io_open (io, argv[optind], 0, 0)) {
		eprintf ("Cannot open '%s'\n", argv[optind]);
		return 1;
	}
	return do_hash (algo, io, bsize, rad);
}
