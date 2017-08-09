#include "config.h"

#include <stdio.h>
#include <stdlib.h>

/* need vcl.h before vrt.h for vmod_evet_f typedef */
#include "vcl.h"
#include "vrt.h"
#include "cache/cache.h"

#include "vtim.h"
#include "vcc_otp_if.h"

#include <syslog.h>
#include <inttypes.h>
#include <mhash.h>
#include <math.h>

static struct sbase32 {
	char c2b[256];  //char -> bin
	char *b2c;      //bin  -> char
	char pad;
} b32;

static void initBase32(){
	const char *p;
	int i;
	b32.pad = '=';
	b32.b2c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

	for (i = 0; i < 256; i++)
		b32.c2b[i] = -1;

	for (p = b32.b2c, i = 0; *p; p++, i++)
		b32.c2b[(int)*p] = (char)i;
	
	b32.c2b[(int)b32.pad] = 0;

}

static int base32ChunkDecode(const char *c,char *buf){
	uint64_t b =0;
	int i,t;
	for(i=0;i<8;i++){
		b<<=5;
		t =b32.c2b[(int)*c++];
		if(t==-1) return 0;
		b |= t;
	}
	buf+=4;
	for(i=0;i<5;i++){
		*buf = (char)(b & 0xff);
		b>>=8;
		buf--;
	}
	return 1;
}

VCL_INT vmod_base32_decode(VRT_CTX,const char*b32txt, char**buf){
	size_t len=strlen(b32txt);
	if(len ==0 || len %8>0){
		return 0;
	}
	len=len/8*5;
	unsigned u;
	u = WS_Reserve(ctx->ws, 0);
	if(len+1 > u){
		WS_Release(ctx->ws, 0);
		WS_MarkOverflow(ctx->ws);
		return 0;
	}
	char *pp, *p;
	pp = p = ctx->ws->f;
	while(1){
		if(!base32ChunkDecode(b32txt,p)){
			WS_Release(ctx->ws, 0);
			return 0;
		}
		b32txt +=8;
		p +=5;
		if(len <= p-pp) break;
	}
	*p = 0;
	p++;
	WS_Release(ctx->ws, p - pp);
	*buf= pp;
	return p - pp;
}


static const char *
vmod_hash_hmac(VRT_CTX,
	hashid hash,void *key,size_t lkey, void *msg,size_t lmsg,bool raw)
{
	size_t blocksize = mhash_get_block_size(hash);

	char *p;
	char *ptmp;
	p    = WS_Alloc(ctx->ws, blocksize * 2 + 1);
	ptmp = p;
	
	unsigned char *mac;
	unsigned u;
	u = WS_Reserve(ctx->ws, 0);
	assert(u > blocksize);
	mac = (unsigned char*)ctx->ws->f;
	
	int i;
	MHASH td;

	assert(msg);
	assert(key);

	assert(mhash_get_hash_pblock(hash) > 0);

	td = mhash_hmac_init(hash, (void *) key, lkey,
		mhash_get_hash_pblock(hash));
	mhash(td, msg, lmsg);
	mhash_hmac_deinit(td,mac);
	if(raw){
		WS_Release(ctx->ws, blocksize);
		return (char *)mac;
	}
	WS_Release(ctx->ws, 0);
	
	for (i = 0; i<blocksize;i++) {
		sprintf(ptmp,"%.2x",mac[i]);
		ptmp+=2;
	}
	return p;
}

VCL_STRING
vmod_otp_gen(VRT_CTX,VCL_STRING b32_secret,uint64_t count, int digit,VCL_ENUM digest){
	char *sec;
	int lsec = vmod_base32_decode(ctx,b32_secret,&sec);
	hashid hash;
	if      (!strcmp(digest, "sha1")){
		hash = MHASH_SHA1;
	}else if(!strcmp(digest, "sha256")){
		hash = MHASH_SHA256;
	}else if(!strcmp(digest, "sha512")){
		hash = MHASH_SHA512;
	}else if(!strcmp(digest, "md5")){
		hash = MHASH_MD5;
	}else{
		hash = MHASH_MD4;
	}
	char ccount[8];
	for(int i=0;i<8;i++){
		ccount[7-i] = (count & (0xff << (i*8))) >> (i*8);
	}
	const char *hmac =vmod_hash_hmac(ctx,hash,sec,lsec,ccount,8,true);
	int offset = hmac[(int)mhash_get_block_size(hash)-1] & 0xf;
	int code   =((hmac[offset    ] & 0x7F) << 24) |
				((hmac[offset + 1] & 0xFF) << 16) |
				((hmac[offset + 2] & 0xFF) << 8 ) |
				 (hmac[offset + 3] & 0xFF);
	char fmt[21];
	sprintf(fmt,"%%0%dd",digit);
	char*p    = WS_Alloc(ctx->ws, digit+ 1);
	sprintf(p,fmt, code % (int)pow(10, digit));
	return p;

}

int __match_proto__(vmod_event_f)
event_function(VRT_CTX, struct vmod_priv *priv, enum vcl_event_e e)
{

	switch (e) {
	case VCL_EVENT_LOAD:
		initBase32();
		break;
	default:
		break;
	}

	return (0);
}


VCL_STRING
vmod_hotp(VRT_CTX,VCL_STRING b32_secret,VCL_INT count, VCL_INT digit,VCL_ENUM digest){
	return vmod_otp_gen(ctx,b32_secret,count,digit,digest);
}

VCL_STRING
vmod_totp(VRT_CTX,VCL_STRING b32_secret,VCL_INT interval, VCL_INT digit,VCL_ENUM digest){
	return vmod_otp_gen(ctx,b32_secret,(uint64_t)floor(ctx->now / interval),digit,digest);
}

VCL_STRING
vmod_totp_settime(VRT_CTX,VCL_STRING b32_secret,VCL_REAL time , VCL_INT interval, VCL_INT digit,VCL_ENUM digest){
	return vmod_otp_gen(ctx,b32_secret,(uint64_t)floor(time / interval),digit,digest);
}

