#include "os_call.c"
#include "disp.c"
#include "string.c"
#include "mt19937ar.c"


#define H_WIN 320                      // ウィンドウの幅
#define W_WIN 448                      // ウィンドウの高さ

#define W2_HODAI 10                    // 砲台の横幅の半分
#define H_HODAI 15                     // 砲台の上面のy座標
#define L_HODAI 5                      // 砲台の下面のy座標
#define V_HODAI 2                      // 砲台の移動速度
#define L_E_BEAM 20                    // 防衛軍のビームの長さ
#define V_E_BEAM 5                     // 防衛軍のビームの速度
#define N_E_BEAM 1                     // 防衛軍のビームの画面上の最大数

#define L_I_BEAM 10                    // インベーダー軍のビームの長さ
#define V_I_BEAM 2                     // インベーダー軍のビームの速度
#define P_I_BEAM 500                   // インベーダー軍のビームの初期発射確率

#define N_I_BEAM 20                    // インベーダー軍のビームの画面上の最大数
#define NXIV 12                        // インベーダー軍の列の数
#define NYIV 4                         // インベーダー軍の行の数
#define V_INVADER 1                    // インベーダー軍の速度

#define NOT_DECIDE 0
#define INVADER 1
#define HUMAN 2

//---- グローバル変数 -------
int xc = 100;                               // マウスのx座標
unsigned char invd_status[NXIV][NYIV];
int invd_x[NXIV][NYIV];
int invd_y[NXIV][NYIV];

int alive_inv=NXIV*NYIV;                    // 生きているインベーダーの数
int inv_vx=V_INVADER;                       // インベーダーの横方向の速度

char e_beam_status[N_E_BEAM];                 // 地球防衛軍のビーム
int e_beam_x[N_E_BEAM];
int e_beam_y0[N_E_BEAM];
int e_beam_y1[N_E_BEAM];
int e_beam_vy[N_E_BEAM];

//beam *p_e_beam1;                            // 地球防衛軍の次に発射可能なビーム
int p_e_beam1;

//beam i_beam[N_I_BEAM];                      // インベーダー軍のビーム
char i_beam_status[N_I_BEAM];
int i_beam_x[N_I_BEAM];
int i_beam_y0[N_I_BEAM];
int i_beam_y1[N_I_BEAM];
int i_beam_vy[N_I_BEAM];

int winner = NOT_DECIDE;

int video[320][14];							// 320x448 ドット
//====================================================================
// main関数
//====================================================================
int main()
{
	int c;

	for(;;){
		initialize();
		scrn_clear();
		cursor_set(25,7);
		printf("シューティングゲーム\n");
		cursor_set(25,9);
		printf("Space ：ビーム発射\n");
		cursor_set(25,10);
		printf("右矢印：砲台右移動\n");
		cursor_set(25,11);
		printf("左矢印：砲台左移動\n");
		ut_getc();
	
		for(;;){
			c = get_APP_chA();				// キー入力
			if(c==' '){
				shoot();					// ビーム発射
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
			else if(c=='s'){				// 画面停止
				ut_getc();
			}
			change_state();					// 状態変化処理
			if(draw()){						// 画面表示
				break;						// ゲーム終了
			}
		}
	}
}

//====================================================================
// 初期化
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

  e_beam_status[0]=1;                       // 砲台にのせる
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
      invd_x[i][j] = 20*(i+1);            // x,yとも20ピクセル間隔
      invd_y[i][j] = H_WIN - NYIV*20+10+20*j;
    }
  }
  winner = NOT_DECIDE;
}


//====================================================================
// 図を描く
//====================================================================
int draw()
{
	int i;
	
	if(winner != NOT_DECIDE){
		draw_result();
		return 1;							// game 終了
	}
	
	memset(video, 0, sizeof(video));
	draw_hodai();         // 砲台を描く関数呼び出し
	draw_e_beam();        // 地球防衛軍のビームを描く関数の呼び出し
	draw_i_beam();        // インベーダー軍のビームを描く関数の呼び出し
	draw_invader();       // インベーダーを描く関数の呼び出し
	
	for(i=0; i<320; i++){
		memcpy((char*)(0xa0000000+4*4+20*4*i), video[i], 4*14);
	}
	return 0;								// normal
}


//====================================================================
// 勝者の表示
//====================================================================
void draw_result()
{
	scrn_clear();
	cursor_set(25,8);
	if(winner==HUMAN)
		printf("=== 貴方の勝ちです\n");
	else
		printf("*** 貴方の負けです\n");
	cursor_set(25,10);
	printf("push enter key\n");
	while(ut_getc() != '\n')
		;
}


//====================================================================
// 地球防衛軍の砲台の描画
//====================================================================
//#define W2_HODAI 10                    // 砲台の横幅の半分
//#define H_HODAI 15                     // 砲台の上面のy座標
//#define L_HODAI 5                      // 砲台の下面のy座標

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
// 地球防衛軍のビーム砲の描画
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
// インベーダー軍のビームの描画
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
// インベーダー軍の描画
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
			if(invd_status[i][j]==1){		// 生きているインベーダーのみ描く
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
// PCが暇なときに実行する．これが実行されると状態が変化する
//====================================================================
void change_state()
{

  if(winner == NOT_DECIDE){
    state_e_beam();     // 地球防衛軍のビームの処理
    state_invader();    // インベーダー軍の処理
    state_i_beam();     // インベーダー軍のビームの処理
  }
}


//====================================================================
// 地球防衛軍のビームの状態の処理
//====================================================================
void state_e_beam()
{
  int i,l,m;
  int st0;
  int rdy;
  int nshoot;                // 発射済みの地球防衛軍の玉の数
  int min_y;                 // 最もしたのミサイルの先のy座標
  int ydis;                  // 最も下のミサイルと発射台の距離

	st0 = 0;
	rdy = 0;
	nshoot = 0;
	min_y = H_WIN+L_E_BEAM;

  for(i=0; i<N_E_BEAM; i++){
    switch(e_beam_status[i]){

    //--------  格納庫にあるビームの処理 ------------------------
    case 0:
      st0=i;                    // 次に発射可能なビームを設定
      break;

    //--------  砲台にあるビームの処理 ------------------------
    case 1:
      e_beam_x[i] = xc;         // x方向に移動
      rdy=1;                    // 砲台にビームがあることを示すフラグをON
      break;

    //--------  すでに発射されたビームの処理 ------------------------
    case 2:
      nshoot++;                         // 発射されているビームをカウント
      e_beam_y0[i] += e_beam_vy[i];     // ビームの移動
      e_beam_y1[i] += e_beam_vy[i];

      // ------ インベーダーにビームが衝突したことを確認して処理 ------
      for(l=0; l<NXIV; l++){    
		for(m=0; m<NYIV; m++){
		  if(invd_status[l][m]==1){
		    if((invd_x[l][m]-8 < e_beam_x[i]) &&
		       (e_beam_x[i] < invd_x[l][m]+8) &&
		       (invd_y[l][m]-4 < e_beam_y0[i]) &&
		       (e_beam_y1[i] < invd_y[l][m]+4)){
		      buzzer(700, 500);
		      invd_status[l][m]=0;            // インベーダーの死亡
		      alive_inv--;                    // 生きているインベーダーの数を-1
		      if(alive_inv==0)winner=HUMAN;
		      e_beam_status[i]=0;             // ビームは格納庫へ
		      e_beam_y0[i]=H_HODAI+L_E_BEAM;  // ビームの初期化
		      e_beam_y1[i]=H_HODAI;
		    }
		  }
		}      
      }


      // ---- 画面から地球防衛軍のビームがはみ出た場合の処理 --------
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


  // --- 地球防衛軍の新たな発射可能なビームの処理 -----
  ydis = min_y-H_HODAI;
  if( (L_E_BEAM*10/4 < ydis) && (rdy==0) && (nshoot<N_E_BEAM) ){
    e_beam_status[st0]=1;
	p_e_beam1 = st0;
  }
}


//====================================================================
// インベーダー軍の状態の処理
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
      if(invd_status[i][j]==1){   // インベーダーの生死のチェック
		invd_x[i][j] += inv_vx;   // インベーダーの横方向移動
		// ---- インベーダー軍のビーム発射の処理 ------
		tmp = MCrnd(P_I_BEAM);
		if(can_attack == 1 && tmp == 0){  // 発射条件
		  for(k=0; k<N_I_BEAM; k++){
		    if(i_beam_status[k] !=2){      // 発射可能なビームを探す
		      i_beam_status[k] =2;         // ビームの発射
		      i_beam_x[k] = invd_x[i][j];
		      i_beam_y0[k] = invd_y[i][j];
		      i_beam_y1[k] = invd_y[i][j]-L_I_BEAM;
		      can_attack=0;
		      break;
		    }
		  }
		}
		// --- インベーダー軍の左右上下の端の座標 -------
		if(invd_x[i][j] < ivmin_x) ivmin_x=invd_x[i][j];   // 左端 
		if(invd_x[i][j] > ivmax_x) ivmax_x=invd_x[i][j];   // 右端
		if(invd_y[i][j] < ivmin_y) ivmin_y=invd_y[i][j];   // 下の端
		if(invd_y[i][j] > ivmax_y) ivmax_y=invd_y[i][j];   // 上の端
      }
    }
  }


  if(ivmin_x < 10) inv_vx = V_INVADER;           // 左端に達したとき
  if(ivmax_x > W_WIN-10) inv_vx = -V_INVADER;    // 右端に達したとき
  
  if((ivmin_x < 10) || (ivmax_x > W_WIN-10)){    // 左右の端に達しとき
    for(i=0; i<NXIV; i++){
      for(j=0; j<NYIV; j++){
	invd_y[i][j] -= 10;                       // 下に降りる
      }
    }
  }
}


//====================================================================
// インベーダー軍のビームの状態の処理
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
		winner=INVADER;            // 地球防衛軍の負け
      }
    }
  }
}


//====================================================================
// マウスイベントの処理
//====================================================================
void mouse_xy(int x, int y)
{
  xc=x;                // マウスのx座標をグローバル変数の xc へ代入
}


//====================================================================
// キーボードイベントの処理
// スペースキーが押されたら地球防衛軍のビームを発射
//====================================================================
void shoot()
{
  //--- スペースキーが押されて，発射可能なビームがあるとき ----
  if(p_e_beam1 != -1){
	buzzer(1000, 50);
	e_beam_status[p_e_beam1] = 2;       // ビームを発射の状態にする
	e_beam_vy[p_e_beam1] = V_E_BEAM;    // ビームの速度を設定
    p_e_beam1 = -1;                 // 発射可能なビームが無い
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
