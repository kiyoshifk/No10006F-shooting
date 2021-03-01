#include "os_call.c"
#include "disp.c"
#include "string.c"
#include "mt19937ar.c"


#define H_WIN 320                      // �E�B���h�E�̕�
#define W_WIN 448                      // �E�B���h�E�̍���

#define W2_HODAI 10                    // �C��̉����̔���
#define H_HODAI 15                     // �C��̏�ʂ�y���W
#define L_HODAI 5                      // �C��̉��ʂ�y���W
#define V_HODAI 2                      // �C��̈ړ����x
#define L_E_BEAM 20                    // �h�q�R�̃r�[���̒���
#define V_E_BEAM 5                     // �h�q�R�̃r�[���̑��x
#define N_E_BEAM 1                     // �h�q�R�̃r�[���̉�ʏ�̍ő吔

#define L_I_BEAM 10                    // �C���x�[�_�[�R�̃r�[���̒���
#define V_I_BEAM 2                     // �C���x�[�_�[�R�̃r�[���̑��x
#define P_I_BEAM 500                   // �C���x�[�_�[�R�̃r�[���̏������ˊm��

#define N_I_BEAM 20                    // �C���x�[�_�[�R�̃r�[���̉�ʏ�̍ő吔
#define NXIV 12                        // �C���x�[�_�[�R�̗�̐�
#define NYIV 4                         // �C���x�[�_�[�R�̍s�̐�
#define V_INVADER 1                    // �C���x�[�_�[�R�̑��x

#define NOT_DECIDE 0
#define INVADER 1
#define HUMAN 2

//---- �O���[�o���ϐ� -------
int xc = 100;                               // �}�E�X��x���W
unsigned char invd_status[NXIV][NYIV];
int invd_x[NXIV][NYIV];
int invd_y[NXIV][NYIV];

int alive_inv=NXIV*NYIV;                    // �����Ă���C���x�[�_�[�̐�
int inv_vx=V_INVADER;                       // �C���x�[�_�[�̉������̑��x

char e_beam_status[N_E_BEAM];                 // �n���h�q�R�̃r�[��
int e_beam_x[N_E_BEAM];
int e_beam_y0[N_E_BEAM];
int e_beam_y1[N_E_BEAM];
int e_beam_vy[N_E_BEAM];

//beam *p_e_beam1;                            // �n���h�q�R�̎��ɔ��ˉ\�ȃr�[��
int p_e_beam1;

//beam i_beam[N_I_BEAM];                      // �C���x�[�_�[�R�̃r�[��
char i_beam_status[N_I_BEAM];
int i_beam_x[N_I_BEAM];
int i_beam_y0[N_I_BEAM];
int i_beam_y1[N_I_BEAM];
int i_beam_vy[N_I_BEAM];

int winner = NOT_DECIDE;

int video[320][14];							// 320x448 �h�b�g
//====================================================================
// main�֐�
//====================================================================
int main()
{
	int c;

	for(;;){
		initialize();
		scrn_clear();
		cursor_set(25,7);
		printf("�V���[�e�B���O�Q�[��\n");
		cursor_set(25,9);
		printf("Space �F�r�[������\n");
		cursor_set(25,10);
		printf("�E���F�C��E�ړ�\n");
		cursor_set(25,11);
		printf("�����F�C�䍶�ړ�\n");
		ut_getc();
	
		for(;;){
			c = get_APP_chA();				// �L�[����
			if(c==' '){
				shoot();					// �r�[������
			}
			else if(c==RIGHT){
				xc += V_HODAI;
				if(xc > W_WIN-1)
					xc = W_WIN-1;
			}
			else if(c==LEFT){
				xc -= V_HODAI;
				if(xc < 0)
					xc = 0;
			}
			else if(c=='s'){				// ��ʒ�~
				ut_getc();
			}
			change_state();					// ��ԕω�����
			if(draw()){						// ��ʕ\��
				break;						// �Q�[���I��
			}
		}
	}
}

//====================================================================
// ������
//====================================================================
void initialize()
{
  int i, j;

	init_genrand(GetTickCount());

  for(i=0; i<N_E_BEAM; i++){
    e_beam_status[i]=0;
    e_beam_y0[i]=H_HODAI+L_E_BEAM;
    e_beam_y1[i]=H_HODAI;
    e_beam_vy[i]=0;
  }

  e_beam_status[0]=1;                       // �C��ɂ̂���
	p_e_beam1 = 0;

  for(i=0; i<N_I_BEAM; i++){
    i_beam_status[i] = 0;
    i_beam_y0[i] = 0;
    i_beam_y1[i] = 0;
    i_beam_vy[i] = V_I_BEAM;
  }

  for(i=0; i<NXIV; i++){
    for(j=0; j<NYIV; j++){
      invd_status[i][j]=1;
      invd_x[i][j] = 20*(i+1);            // x,y�Ƃ�20�s�N�Z���Ԋu
      invd_y[i][j] = H_WIN - NYIV*20+10+20*j;
    }
  }
  winner = NOT_DECIDE;
}


//====================================================================
// �}��`��
//====================================================================
int draw()
{
	int i;
	
	if(winner != NOT_DECIDE){
		draw_result();
		return 1;							// game �I��
	}
	
	memset(video, 0, sizeof(video));
	draw_hodai();         // �C���`���֐��Ăяo��
	draw_e_beam();        // �n���h�q�R�̃r�[����`���֐��̌Ăяo��
	draw_i_beam();        // �C���x�[�_�[�R�̃r�[����`���֐��̌Ăяo��
	draw_invader();       // �C���x�[�_�[��`���֐��̌Ăяo��
	
	for(i=0; i<320; i++){
		memcpy((char*)(0xa0000000+4*4+20*4*i), video[i], 4*14);
	}
	return 0;								// normal
}


//====================================================================
// ���҂̕\��
//====================================================================
void draw_result()
{
	scrn_clear();
	cursor_set(25,8);
	if(winner==HUMAN)
		printf("=== �M���̏����ł�\n");
	else
		printf("*** �M���̕����ł�\n");
	cursor_set(25,10);
	printf("push enter key\n");
	while(ut_getc() != '\n')
		;
}


//====================================================================
// �n���h�q�R�̖C��̕`��
//====================================================================
//#define W2_HODAI 10                    // �C��̉����̔���
//#define H_HODAI 15                     // �C��̏�ʂ�y���W
//#define L_HODAI 5                      // �C��̉��ʂ�y���W

void draw_hodai()
{
	int x, y;
	
	for(y=L_HODAI; y<=H_HODAI; y++){
		for(x=xc-W2_HODAI; x<=xc+W2_HODAI; x++){
			Psetx(x,y, 1);
		}
	}
}


//====================================================================
// �n���h�q�R�̃r�[���C�̕`��
//====================================================================
void draw_e_beam()
{
	int i, y;
	
	for(i=0; i<N_E_BEAM; i++){
		if(e_beam_status[i] != 0){
			for(y=e_beam_y1[i]; y<=e_beam_y0[i]; y++){
				Psetx(e_beam_x[i], y, 1);
			}
		}
	}
}


//====================================================================
// �C���x�[�_�[�R�̃r�[���̕`��
//====================================================================
void draw_i_beam()
{
	int i, y;
	
	for(i=0; i<N_I_BEAM; i++){
		if(i_beam_status[i] == 2){
			for(y=i_beam_y1[i]; y<=i_beam_y0[i]; y++){
				Psetx(i_beam_x[i], y, 1);
			}
		}
	}
}


//====================================================================
// �C���x�[�_�[�R�̕`��
//====================================================================
const char draw_invader_data[15][17]={
	0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,
	0,1,0,0,1,0,0,0,0,0,0,0,1,0,0,1,0,
	0,1,0,0,0,1,1,1,1,1,1,1,0,0,0,1,0,
	0,1,0,0,1,1,1,1,1,1,1,1,1,0,0,1,0,
	0,1,0,1,1,0,0,1,1,1,0,0,1,1,0,1,0,
	0,1,1,1,1,0,0,1,1,1,0,0,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
	0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,
	0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,
	0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
};

void draw_invader()
{
	int i, j, x, y, xx, yy;
	
	for(i=0; i<NXIV; i++){
		for(j=0; j<NYIV; j++){
			if(invd_status[i][j]==1){		// �����Ă���C���x�[�_�[�̂ݕ`��
				xx = invd_x[i][j]+8;
				yy = invd_y[i][j]+6;
				for(y=0; y<15; y++){
					for(x=0; x<17; x++){
						if(draw_invader_data[y][x])
							Psetx(xx-x, yy-y, 1);
					}
				}
			}
		}
	}
}

//====================================================================
// PC���ɂȂƂ��Ɏ��s����D���ꂪ���s�����Ə�Ԃ��ω�����
//====================================================================
void change_state()
{

  if(winner == NOT_DECIDE){
    state_e_beam();     // �n���h�q�R�̃r�[���̏���
    state_invader();    // �C���x�[�_�[�R�̏���
    state_i_beam();     // �C���x�[�_�[�R�̃r�[���̏���
  }
}


//====================================================================
// �n���h�q�R�̃r�[���̏�Ԃ̏���
//====================================================================
void state_e_beam()
{
  int i,l,m;
  int st0;
  int rdy;
  int nshoot;                // ���ˍς݂̒n���h�q�R�̋ʂ̐�
  int min_y;                 // �ł������̃~�T�C���̐��y���W
  int ydis;                  // �ł����̃~�T�C���Ɣ��ˑ�̋���

	st0 = 0;
	rdy = 0;
	nshoot = 0;
	min_y = H_WIN+L_E_BEAM;

  for(i=0; i<N_E_BEAM; i++){
    switch(e_beam_status[i]){

    //--------  �i�[�ɂɂ���r�[���̏��� ------------------------
    case 0:
      st0=i;                    // ���ɔ��ˉ\�ȃr�[����ݒ�
      break;

    //--------  �C��ɂ���r�[���̏��� ------------------------
    case 1:
      e_beam_x[i] = xc;         // x�����Ɉړ�
      rdy=1;                    // �C��Ƀr�[�������邱�Ƃ������t���O��ON
      break;

    //--------  ���łɔ��˂��ꂽ�r�[���̏��� ------------------------
    case 2:
      nshoot++;                         // ���˂���Ă���r�[�����J�E���g
      e_beam_y0[i] += e_beam_vy[i];     // �r�[���̈ړ�
      e_beam_y1[i] += e_beam_vy[i];

      // ------ �C���x�[�_�[�Ƀr�[�����Փ˂������Ƃ��m�F���ď��� ------
      for(l=0; l<NXIV; l++){    
		for(m=0; m<NYIV; m++){
		  if(invd_status[l][m]==1){
		    if((invd_x[l][m]-8 < e_beam_x[i]) &&
		       (e_beam_x[i] < invd_x[l][m]+8) &&
		       (invd_y[l][m]-4 < e_beam_y0[i]) &&
		       (e_beam_y1[i] < invd_y[l][m]+4)){
		      buzzer(700, 500);
		      invd_status[l][m]=0;            // �C���x�[�_�[�̎��S
		      alive_inv--;                    // �����Ă���C���x�[�_�[�̐���-1
		      if(alive_inv==0)winner=HUMAN;
		      e_beam_status[i]=0;             // �r�[���͊i�[�ɂ�
		      e_beam_y0[i]=H_HODAI+L_E_BEAM;  // �r�[���̏�����
		      e_beam_y1[i]=H_HODAI;
		    }
		  }
		}      
      }


      // ---- ��ʂ���n���h�q�R�̃r�[�����͂ݏo���ꍇ�̏��� --------
      if(H_WIN+L_E_BEAM < e_beam_y0[i]){
		e_beam_status[i] = 0;
		e_beam_y0[i] = H_HODAI+L_E_BEAM;
		e_beam_y1[i] = H_HODAI;
		e_beam_vy[i] = 0;
      }
      if(e_beam_y0[i] < min_y) min_y=e_beam_y0[i];
      break;
    default:
      printf("e_beam status error!!\n");
      for(;;)
        ;
    }
  }


  // --- �n���h�q�R�̐V���Ȕ��ˉ\�ȃr�[���̏��� -----
  ydis = min_y-H_HODAI;
  if( (L_E_BEAM*10/4 < ydis) && (rdy==0) && (nshoot<N_E_BEAM) ){
    e_beam_status[st0]=1;
	p_e_beam1 = st0;
  }
}


//====================================================================
// �C���x�[�_�[�R�̏�Ԃ̏���
//====================================================================
void state_invader()
{
	int tmp;
  int i, j, k;
  int ivmin_x, ivmax_x;
  int ivmin_y, ivmax_y;
  int can_attack;

	ivmin_x = W_WIN; ivmax_x = 0;
	ivmin_y = H_WIN; ivmax_y = 0;

  for(i=0; i<NXIV; i++){
    can_attack=1;
    for(j=0; j<NYIV; j++){
      if(invd_status[i][j]==1){   // �C���x�[�_�[�̐����̃`�F�b�N
		invd_x[i][j] += inv_vx;   // �C���x�[�_�[�̉������ړ�
		// ---- �C���x�[�_�[�R�̃r�[�����˂̏��� ------
		tmp = MCrnd(P_I_BEAM);
		if(can_attack == 1 && tmp == 0){  // ���ˏ���
		  for(k=0; k<N_I_BEAM; k++){
		    if(i_beam_status[k] !=2){      // ���ˉ\�ȃr�[����T��
		      i_beam_status[k] =2;         // �r�[���̔���
		      i_beam_x[k] = invd_x[i][j];
		      i_beam_y0[k] = invd_y[i][j];
		      i_beam_y1[k] = invd_y[i][j]-L_I_BEAM;
		      can_attack=0;
		      break;
		    }
		  }
		}
		// --- �C���x�[�_�[�R�̍��E�㉺�̒[�̍��W -------
		if(invd_x[i][j] < ivmin_x) ivmin_x=invd_x[i][j];   // ���[ 
		if(invd_x[i][j] > ivmax_x) ivmax_x=invd_x[i][j];   // �E�[
		if(invd_y[i][j] < ivmin_y) ivmin_y=invd_y[i][j];   // ���̒[
		if(invd_y[i][j] > ivmax_y) ivmax_y=invd_y[i][j];   // ��̒[
      }
    }
  }


  if(ivmin_x < 10) inv_vx = V_INVADER;           // ���[�ɒB�����Ƃ�
  if(ivmax_x > W_WIN-10) inv_vx = -V_INVADER;    // �E�[�ɒB�����Ƃ�
  
  if((ivmin_x < 10) || (ivmax_x > W_WIN-10)){    // ���E�̒[�ɒB���Ƃ�
    for(i=0; i<NXIV; i++){
      for(j=0; j<NYIV; j++){
	invd_y[i][j] -= 10;                       // ���ɍ~���
      }
    }
  }
}


//====================================================================
// �C���x�[�_�[�R�̃r�[���̏�Ԃ̏���
//====================================================================
void state_i_beam()
{
  int i;

  for(i=0; i<N_I_BEAM; i++){
    if(i_beam_status[i] ==2){
      i_beam_y0[i] -= i_beam_vy[i];
      i_beam_y1[i] -= i_beam_vy[i];
   
      if(i_beam_y1[i] < 0) i_beam_status[i]=0;

      if((xc-W2_HODAI < i_beam_x[i]) &&
		 (i_beam_x[i] < xc+W2_HODAI) &&
		 (L_HODAI < i_beam_y0[i]) &&
		 (i_beam_y1[i] < H_HODAI)){
		buzzer(200, 1000);
		winner=INVADER;            // �n���h�q�R�̕���
      }
    }
  }
}


//====================================================================
// �}�E�X�C�x���g�̏���
//====================================================================
void mouse_xy(int x, int y)
{
  xc=x;                // �}�E�X��x���W���O���[�o���ϐ��� xc �֑��
}


//====================================================================
// �L�[�{�[�h�C�x���g�̏���
// �X�y�[�X�L�[�������ꂽ��n���h�q�R�̃r�[���𔭎�
//====================================================================
void shoot()
{
  //--- �X�y�[�X�L�[��������āC���ˉ\�ȃr�[��������Ƃ� ----
  if(p_e_beam1 != -1){
	buzzer(1000, 50);
	e_beam_status[p_e_beam1] = 2;       // �r�[���𔭎˂̏�Ԃɂ���
	e_beam_vy[p_e_beam1] = V_E_BEAM;    // �r�[���̑��x��ݒ�
    p_e_beam1 = -1;                 // ���ˉ\�ȃr�[��������
  }
}

/********************************************************************************/
/*		MCrnd																	*/
/********************************************************************************/
long MCrnd(long i)
{
	return genrand_int31() % i;
}
/********************************************************************************/
/*		Psetx																	*/
/********************************************************************************/
void Psetx(int x, int y, int c)
{
	int bit;
	int xx;
	
	if(x<0 || x>=448 || y<0 || y>=320)
		return;							// out of range
	y = 319-y;
	xx = (x>>5) & 0xff;
	bit = 0x80000000 >> (x & 0x1f);
	if(c)
		video[y][xx] |= bit;
	else
		video[y][xx] &= ~bit;
}
