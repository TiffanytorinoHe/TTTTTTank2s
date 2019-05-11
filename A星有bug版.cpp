// Tank2S 样例
// 作者：recordmp3
#include <stack>
#include <set>
#include <string>
#include <iostream>
#include <ctime>
#include <cmath>
#include <list>
#include <cstring>
#include "jsoncpp/json.h"
#define MAX 1000000
#define Mweight 100
#define Dweight 1000
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
struct Point
{
    int x,y;
    int G,H,F;
    Point* parent;
    Point(int tx,int ty):x{tx},y{ty},G{0},H{0},F{0},parent{nullptr} {}
};
std::list<Point*> openList;
std::list<Point*> closeList;
bool ok(int x,int y)// can a tank steps in?
{
	return x>=0&&x<=8&&y>=0&&y<=8&&(~state[y][x] & steel)&&(~state[y][x] & water)&&(~state[y][x] & brick);
}
int shoot_cnt[2];
bool valid(int id,int action)//is tank id's action valid?
{
    if(action<-1 ||action>7) return false;
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
bool stupidMove(int x,int y)
{
    if((state[y][x]&steel)||(state[y][x]&water)||!(x>=0&&x<=8&&y>=0&&y<=8))
        return true;
    for(int i=0;i<2;i++)
	{
		if(enemy_position[i*2]>=0)//can not step into a tank's block (although tanks can overlap inadventently)
		{
			if((x - enemy_position[i*2] == 0) && (y - enemy_position[i*2+1] == 0))
			return true;
		}
		if(self_position[i*2]>=0)
		{
			if((x - self_position[i*2] == 0) && (y - self_position[i*2+1] == 0))
			return true;
		}
	}
	return false;
}

int DangerLevel(int id,int x,int y)
{
    for(int i=0;i<2;i++){
        if(enemy_position[i*2]==x&&enemy_position[(1-i)*2+1]==y)
            return 3;
    }
    if(enemy_position[0]==x&&enemy_position[2]==x)
        return 2;
    if(enemy_position[1]==y&&enemy_position[3]==y)
        return 2;
    for(int i=0;i<2;i++){
    if(enemy_position[i*2]==x){
        int y0=std::min(y,enemy_position[i*2+1]);
        int y1=std::max(y,enemy_position[i*2+1]);
        for(int sy=y0+1;sy<y1;sy++){
            if((!!(state[sy][x] & steel))||(!!(state[sy][x] & brick)))
                return 1;
        }
        if(shoot_cnt[id]==0)
            return -1;
        else return 3;
    }
    if(enemy_position[i*2+1]==y){
        int x0=std::min(x,enemy_position[i*2]);
        int x1=std::max(x,enemy_position[i*2]);
        for(int sx=x0+1;sx<x1;sx++){
            if((!!(state[y][sx] & steel))||(!!(state[y][sx] & brick)))
                return 1;
        }
        if(shoot_cnt[id]==0)
            return -1;
        else return 3;
    }
    }
    return 0;
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

int countValue(int id,int x, int y, int goalX=4,int goalY=8-8*myside)//越小越好
{
    int M=std::abs(goalY-y)+std::abs(goalX-x)+(int)(state[y][x] & brick);
    int D=(int)(DangerLevel(id,x,y));
    return M*Mweight+D*Dweight;
}

bool TimeUp(clock_t start)
{
    clock_t End=clock();
    return ((double)(End-start)/CLOCKS_PER_SEC>0.8);
}
namespace Basic
{
    int RunAway(int id)
    {
        int action=rand()%3+1;
        while(!stupidA(id,action)){
            action=rand()%3+1;
        }
        return action;
    }
    int Attack(int id)
    {
        int x0=self_position[id*2];
        int y0=self_position[id*2+1];
        if(x0==4)
            return (1-id)*2+4;
        if(y0==8-8*myside)
            return (x0>4?3:5);
        for(int i=0;i<2;i++){
        if(enemy_position[2*i]==x0 && enemy_position[2*i+1]<y0) return 4;
        if(enemy_position[2*i]==x0 && enemy_position[2*i+1]>y0) return 6;
        if(enemy_position[2*i+1]==y0 && enemy_position[2*i]<x0) return 7;
        if(enemy_position[2*i+1]==y0 && enemy_position[2*i]>x0) return 5;
        }
        return RunAway(id);
    }
    int EmergentCase(int id)
    {
        int d=DangerLevel(id,self_position[id*2],self_position[id*2+1]);
        if(d>=2)
          return RunAway(id);
        if(d==-1)
          return Attack(id);
        return 0;
    }
}

namespace Invasion
{
   Point* InvadeTarget(int id)
   {
        if(enemy_position[0]!=-1&&enemy_position[2]!=-1){
            Point* goal=new Point(4,8-8*myside);
            return goal;
        }
        int target=enemy_position[0]==-1?1:0;
        int tx=enemy_position[target*2];
        int ty=enemy_position[target*2+1];
        int x0=tx,y0=ty,m=MAX;
        for(int i=tx-1;i<=tx+1;i++){
            for(int j=ty-1;j<=ty+1;j++){
                if(!ok(i,j)) continue;
                int temp=countValue(id,tx,ty);
                if(temp<m){
                    x0=i;
                    y0=j;
                    m=temp;
                }
            }
        }
        Point* goal=new Point(x0,y0);
        return goal;
   }
}
namespace Astar
{
    Point* getLeastFpoint()
    {
        if(!openList.empty())
        {
            auto resPoint=openList.front();
            for(auto &point:openList)
			if(point->F<resPoint->F)
				resPoint=point;
            return resPoint;
        }
        return nullptr;
    }
    Point* isInList(const std::list<Point *> &list,const Point *point)
    {
	//判断某个节点是否在列表中，这里不能比较指针，因为每次加入列表是新开辟的节点，只能比较坐标
        for(auto p:list)
		if(p->x==point->x&&p->y==point->y)
			return p;
        return nullptr;
    }

    Point* AstarFindWay(int id, int goalX, int goalY)
    {
        openList.push_back(new Point(self_position[2*id],self_position[2*id+1]));
        while(!openList.empty())
        {
            auto curPoint=getLeastFpoint(); //找到F值最小的点
            openList.remove(curPoint); //从开启列表中删除
            closeList.push_back(curPoint); //放到关闭列表
            for(int i=0;i<4;i++){
               int tx=self_position[2*id]+px[i];
               int ty=self_position[2*id+1]+py[i];
               if(stupidMove(tx,ty)) continue;
               Point* target=new Point(tx,ty);
               Point* searchAnswer=isInList(openList,target);
                //2,对某一个格子，如果它不在开启列表中，加入到开启列表，设置当前格为其父节点，计算F G H D
                if(!searchAnswer)
                {
                    target->parent=curPoint;
                    target->G=(target->parent==nullptr?0:target->parent->G)+1;
                    target->H=countValue(id,target->x,target->y,goalX,goalY);
                    target->F=target->G+target->H;
                    openList.push_back(target);
                }
            //3，对某一个格子，它在开启列表中，计算G值, 如果比原来的大, 就什么都不做, 否则设置它的父节点为当前点,并更新G和F
                else
                {
                    int tempG=(curPoint==nullptr?0:curPoint->G)+1;
                    int tempH=countValue(id,target->x,target->y,goalX,goalY);
                    if(tempG+tempH<searchAnswer->G+searchAnswer->H)
                    {
                        searchAnswer->parent=curPoint;
                        searchAnswer->G=tempG;
                        searchAnswer->H=tempH;
                        searchAnswer->F=tempG+tempH;
                    }
                }
            Point* goal=new Point(goalX,goalY);
			Point* resPoint=isInList(openList,goal);
			if(resPoint)
				return resPoint; //返回列表里的节点指针，不要用原来传入的endpoint指针，因为发生了深拷贝
		}
	}
        return nullptr;
    }
    int GetNextAction(int id,int goalX,int goalY)
    {
        Point *result=AstarFindWay(id, goalX, goalY);
        Point *next=nullptr;
        int x0=self_position[id*2];
        int y0=self_position[id*2+1];
        while(!(result->x==x0&&result->y==y0) && result!=nullptr)
        {
            next=result;
            result=result->parent;
        }
    // 清空临时开闭列表，防止重复执行GetNextAction导致结果异常
        openList.clear();
        closeList.clear();
        if(next->x==x0-1){
            return 3+(int)(state[y0][next->x] & brick)*4;
        }
        if(next->x==x0+1){
            return 1+(int)(state[y0][next->x] & brick)*4;
        }
        if(next->y==y0-1){
            return (int)(state[next->y][x0] & brick)*4;
        }
        if(next->y==y0+1){
            return 2+(int)(state[next->y][x0] & brick)*4;
        }
    }
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
		/*action = Attack(i);
         while(!valid(i, action) || (!!(action>=4)) + shoot_cnt[i] >= 2
         ||(!TimeUp(start)&&(stupidA(i,action)||stupidB(i,action)))){
            action = rand() % 9 - 1;
         }*/
        int e=Basic::EmergentCase(i);
        if(e!=0) action=e;
        else{
            Point* tempP=Invasion::InvadeTarget(i);
            action=Astar::GetNextAction(i,tempP->x,tempP->y);
        }
		a[i] = action;
		output.append(action);
	}
	cout << ",,"<<"\n";
    Json::FastWriter writer;
    cout<<writer.write(output);
    return 0;
}

