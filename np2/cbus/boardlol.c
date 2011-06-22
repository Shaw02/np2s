#include	"compiler.h"
#include	"pccore.h"
#include	"iocore.h"
#include	"cbuscore.h"
#include	"boardlol.h"
#include	"sound.h"
#include	"fmboard.h"
#include	"s98.h"

/**
 * SNE Little Orchestra
 * YM2203(OPN) with panning
 *
 */
static void IOOUTCALL opn_o188(UINT port, REG8 dat) {

	opn.addr = dat;
	opn.data = dat;
	(void)port;
}

static void IOOUTCALL opn_o18a(UINT port, REG8 dat) {

	UINT	addr;

	if ((opn.addr & 0xb4) == 0xb4)
		return;

	opn.data = dat;
	addr = opn.addr;
	S98_put(NORMAL2608, addr, dat);
	if (addr < 0x10) {
		if (addr != 0x0e) {
			psggen_setreg(&psg1, addr, dat);
		}
	}
	else if (addr < 0x100) {
		if (addr < 0x30) {
			if (addr == 0x28) {
				if ((dat & 0x0f) < 3) {
					opngen_keyon(dat & 0x0f, dat);
				}
			}
			else {
				fmtimer_setreg(addr, dat);
				if (addr == 0x27) {
					opnch[2].extop = dat & 0xc0;
				}
			}
		}
		else if (addr < 0xc0) {
			opngen_setreg(0, addr, dat);
		}
		opn.reg[addr] = dat;
	}
	(void)port;
}

static REG8 IOINPCALL opn_i188(UINT port) {

	(void)port;
	return(fmtimer.status);
}

static REG8 IOINPCALL opn_i18a(UINT port) {

	UINT	addr;

	addr = opn.addr;
	if (addr == 0x0e) {
		return(fmboard_getjoy(&psg1));
	}
	else if (addr < 0x10) {
		return(psggen_getreg(&psg1, addr));
	}
	(void)port;
	return(opn.data);
}

// ----

static const IOOUT opn_o[4] = {
			opn_o188,	opn_o18a,	NULL,		NULL};

static const IOINP opn_i[4] = {
			opn_i188,	opn_i18a,	NULL,		NULL};

static void psgpanset(PSGGEN psg) {
	// SSG��R�Œ�
	psggen_setpan(psg, 0, 1);
	psggen_setpan(psg, 1, 1);
	psggen_setpan(psg, 2, 1);
}

void boardlol_reset(const NP2CFG *pConfig) {

	opngen_setcfg(3, OPN_STEREO | 0x007);
	fmtimer_reset(pConfig->snd26opt & 0xc0);
	soundrom_loadex(pConfig->snd26opt & 7, OEMTEXT("26"));
	opn.base = (pConfig->snd26opt & 0x10)?0x000:0x100;
	opngen_setreg(0, 0xb4, 1 << 6);
	opngen_setreg(0, 0xb5, 1 << 6);
	opngen_setreg(0, 0xb6, 1 << 6);
}

void boardlol_bind(void) {
	psgpanset(&psg1);
	fmboard_fmrestore(0, 0);
	opngen_setreg(0, 0xb4, 1 << 7);		// OPN��L�Œ�
	opngen_setreg(0, 0xb5, 1 << 7);
	opngen_setreg(0, 0xb6, 1 << 7);
	psggen_restore(&psg1);
	sound_streamregist(&opngen, (SOUNDCB)opngen_getpcm);
	sound_streamregist(&psg1, (SOUNDCB)psggen_getpcm);
	cbuscore_attachsndex(0x188 - opn.base, opn_o, opn_i);
}

