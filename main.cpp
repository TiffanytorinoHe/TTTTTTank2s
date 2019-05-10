// Tank2S 样例
// 作者：recordmp3
#include <stack>
#include <set>
#include <string>
#include <iostream>
#include <ctime>
#include <cmath>
#include <cstring>
#include "jsoncpp/json.h"

using std::string;
using std::cin;
using std::cout;
using std::endl;
using std::getline;


const int none = 0, brick = 1, forest = 2, steel = 4,water = 8;
int state[10][10];
int enemy_position[4];//x0,y0,x1,y1;
int  self_position[4];//x0,y0,x1,y1;
int myside;
int px[4] = {0,1,0,-1};
int py[4] = {-1,0,1,0};
bool ok(int x,int y)// can a tank steps in?
{
	return x>=0&&x<=8&&y>=0&&y<=8&&(~state[y][x] & steel)&&(~state[y][x] & water)&&(~state[y][x] & brick);
}
int shoot_cnt[2];
bool valid(int id,int action)//is tank id's action valid?
{
	if(self_position[id*2+1] == (myside) * 8)
	{
		if(self_position[id*2] < 4 && action == 5)return false;
		if(self_position[id*2] > 4 && action == 7)return false;// prevent to commit suiside
	}
	if(action == -1 || action >= 4)return true;
	int xx = self_position[id * 2] + px[action];
	int yy = self_position[id*2+1] + py[action];
	if(!ok(xx,yy))return false;
	for(int i=0;i<2;i++)
	{
		if(enemy_position[i*2]>=0)//can not step into a tank's block (although tanks can overlap inadventently)
		{
			if((xx - enemy_position[i*2] == 0) && (yy - enemy_position[i*2+1] == 0))
			return false;
		}
		if(self_position[i*2]>=0)
		{
			if((xx - self_position[i*2] == 0) && (yy - self_position[i*2+1] == 0))
			return false;
		}
	}

	return true;
}

bool stupidA(int id, int action)
{
    for(int i=0;i<2;i++){
        bool s=true;
        if(enemy_position[i*2]>=0){
            if(self_position[id*2]==enemy_position[i*2] && (action==0||action==2)){
                int x0=self_position[id*2];
                int y0=std::min(self_position[id*2+1],enemy_position[i*2+1]);
                int y1=std::max(self_position[id*2+1],enemy_position[i*2+1]);
                for(int y=y0+1;y<y1;y++){
                    if((!!(state[y][x0] & steel))||(!!(state[y][x0] & brick)))
                        s=false;
                }
                if(s) return true;
                s=true;
            }
            if(self_position[id*2+1]==enemy_position[i*2+1] && (action==1||action==3)){
                int y0=self_position[id*2+1];
                int x0=std::min(self_position[id*2],enemy_position[i*2]);
                int x1=std::max(self_position[id*2],enemy_position[i*2]);
                for(int x=x0+1;x<x1;x++){
                    if((!!(state[y0][x] & steel))||(!!(state[y0][x] & brick)))
                        s=false;
                }
                if(s) return true;
            }
        }
    }
    return false;
}
bool stupidB(int id,int action)
{
    int x0=self_position[id*2];
    int y0=self_position[id*2+1];
    switch(action){
    case 0:
    case 1:
    case 2:
    case 3:
        return false;
    case 4:
    case 6:
        if(action==4){
        for(int y=y0-1;y>=0;y--){
            if(state[y][x0] & brick) return false;
            if((enemy_position[0]==x0&&enemy_position[1]==y)||(enemy_position[2]==x0&&enemy_position[3]==y))
                return false;
            if(state[y][x0] & steel) return true;
        }
        return true;
        }
        if(action==6){
            for(int y=y0+1;y<=8;y++){
            if(state[y][x0] & brick) return false;
            if((enemy_position[0]==x0&&enemy_position[1]==y)||(enemy_position[2]==x0&&enemy_position[3]==y))
                return false;
            if(state[y][x0] & steel) return true;
        }
        return true;
        }
    case 5:
    case 7:
        if(action==5){
            for(int x=x0+1;x<=8;x++){
            if(state[y0][x] & brick) return false;
            if((enemy_position[0]==x&&enemy_position[1]==y0)||(enemy_position[2]==x&&enemy_position[3]==y0))
                return false;
            if(state[y0][x] & steel) return true;
        }
        return true;
        }
        if(action==7){
            for(int x=x0-1;x>=0;x--){
            if(state[y0][x] & brick) return false;
            if((enemy_position[0]==x&&enemy_position[1]==y0)||(enemy_position[2]==x&&enemy_position[3]==y0))
                return false;
            if(state[y0][x] & steel) return true;
        }
        return true;
        }
        default:
        return true;
    }
}
bool TimeUp(clock_t start)
{
    clock_t End=clock();
    return ((double)(End-start)/CLOCKS_PER_SEC>0.8);
}
void init()
{
	memset(enemy_position,-1,sizeof(enemy_position));
	state[0][4] = state[8][4] = steel;
}
int main()
{
    Json::Reader reader;
    Json::Value input, temp, all, output;
    #ifdef _BOTZONE_ONLINE
    reader.parse(cin, all);
    #else
    char *s = "{\"requests\":[{\"brickfield\":[4223017,4412944,77623312],\"forestfield\":[70519808,129499375,98657],\"mySide\":0,\"steelfield\":[524288,305408,128],\"waterfield\":[16909184,0,918020]},{\"action\":[0,0],\"destroyed_blocks\":[],\"destroyed_tanks\":[],\"final_enemy_positions\":[-1,-1,2,7]},{\"action\":[-2,3],\"destroyed_blocks\":[],\"destroyed_tanks\":[],\"final_enemy_positions\":[-1,-1,1,7]}],\"responses\":[[-1,5],[6,2]]}";
    reader.parse(s,all);
	#endif

	init();
	int seed = 0;
	input = all["requests"];
	for(int i=0;i < input.size();i++)
	{
		if(i == 0)// read in the map information
		{
			myside = input[0u]["mySide"].asInt();
			if(!myside)
			{
				self_position[0] = 2;
				self_position[1] = 0;
				self_position[2] = 6;
				self_position[3] = 0;
			}
			else
			{
				self_position[0] = 6;
				self_position[1] = 8;
				self_position[2] = 2;
				self_position[3] = 8;
			}
			for(unsigned j=0;j<3;j++)
			{
				int x = input[0u]["brickfield"][j].asInt();
				seed ^= x;
				for(int k=0;k<27;k++)state[j*3 + k/9][k%9] |= (!!((1<<k)&x)) * brick;
			}
			for(unsigned j=0;j<3;j++)
			{
				int x = input[0u]["forestfield"][j].asInt();
				for(int k=0;k<27;k++)state[j*3 + k/9][k%9] |= (!!((1<<k)&x)) * forest;
			}
			for(unsigned j=0;j<3;j++)
			{
				int x = input[0u]["steelfield"][j].asInt();
				for(int k=0;k<27;k++)state[j*3 + k/9][k%9] |= (!!((1<<k)&x)) * steel;
			}
			for(unsigned j=0;j<3;j++)
			{
				int x = input[0u]["waterfield"][j].asInt();
				for(int k=0;k<27;k++)state[j*3 + k/9][k%9] |= (!!((1<<k)&x)) * water;
			}
		}
		else// update enemy_position
		{
			for(int j=0;j<4;j++)enemy_position[j] = input[i]["final_enemy_positions"][j].asInt(),seed += enemy_position[j], seed ^= enemy_position[j];
			for(int j=0;j<input[i]["destroyed_blocks"].size();j+=2)
			{
				int x = input[i]["destroyed_blocks"][j  ].asInt();
				int y = input[i]["destroyed_blocks"][j+1].asInt();
				seed ^= x^y;
				state[y][x] = 0;
			}
		}
	}


	input = all["responses"];
	for(int i=0;i<input.size();i++)//update self state
	{
		for(int j=0;j<2;j++)
		{
			int x = input[i][j].asInt();
			seed ^= x * 2354453456;
			if(x >= 4)shoot_cnt[j] ++;
			else shoot_cnt[j] = 0;
			if(x == -1 || x >= 4)continue;
			self_position[2*j] += px[x];
			self_position[2*j+1] += py[x];
		}
	}
	output = Json::Value(Json::arrayValue);
	srand( self_position[0]^self_position[2]^enemy_position[1]^enemy_position[3]^seed^565646435);
	int a[2];
	clock_t start=clock();
	for(int i=0;i<2;i++)// sample valid action for 2 tanks
	{
		int action;
		do{
			action = rand() % 9 - 1;
		}while(!valid(i, action) || (!!(action>=4)) + shoot_cnt[i] >= 2
         ||(!TimeUp(start)&&(stupidA(i,action)||stupidB(i,action))));
		a[i] = action;
		output.append(action);
	}
    Json::FastWriter writer;
    cout<<writer.write(output);
    return 0;
}

