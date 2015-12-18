#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <fstream>
#include "PlanetWars.h"
using namespace std;

#define pii pair<int,int>
#define vi vector<int>
#define vpii vector<pair<int,int> >

vi MyNumShips;

struct future
{
  int id,owner;
  vpii pred;
};

vector<future> neutral_futureplanets,my_futureplanets,enemy_futureplanets;
vi my_urgent,neutral_urgent,enemy_urgent;

vector<int> neutral_notto_attack;



ofstream out;
int TURN=0;
bool flag=false;

// The DoTurn function is where your code goes. The PlanetWars object contains
// the state of the game, including information about all planets and fleets
// that currently exist. Inside this function, you issue orders using the
// pw.IssueOrder() function. For example, to send 10 ships from planet 3 to
// planet 8, you would say pw.IssueOrder(3, 8, 10).
//
// There is already a basic strategy in place here. You can use it as a
// starting point, or you can throw it out entirely and replace it with your
// own. Check out the tutorials and articles on the contest website at
// http://www.ai-contest.com/resources.

bool Attack = false;
int NUMBER_OF_PRED=20;
int perimeter;
int total_planets;
vector<Planet> idPlanet;
std::vector<int> MyPlanetsSpareShips;

void initialize(const PlanetWars& pw){
  total_planets=pw.Planets().size();
  vector<Planet> planets=pw.Planets();
  Planet tmp=planets[0];
  idPlanet.resize(total_planets,tmp);
  for(int i=0;i<total_planets;i++){
      idPlanet[planets[i].PlanetID()]=planets[i];
  }

}
int ShipsToPlanetBeforeTurn(const PlanetWars& pw,int IDPlanet,int BeforeTurn, int IDPlayer) {
  vector<Fleet> allFleets = pw.Fleets();
  int TotalIncomingFleets = 0;
  for(int i=0;i<allFleets.size();i++) {
    Fleet fleet = allFleets[i];
    if (fleet.DestinationPlanet() != IDPlanet || fleet.TurnsRemaining() > BeforeTurn)
      continue;
    if (fleet.Owner() == IDPlayer)
      TotalIncomingFleets += fleet.NumShips();
    // else if (IDPlayer != 0)
    //   TotalIncomingFleets -= fleet.NumShips();
    else
      TotalIncomingFleets -= fleet.NumShips();
  }
  return TotalIncomingFleets;
}


int ShipsToPlanet(const PlanetWars& pw,int IDPlanet,int turnNumber, int IDPlayer) {
  vector<Fleet> allFleets = pw.Fleets();
  int TotalIncomingFleets = 0;
  for(int i=0;i<allFleets.size();i++) {
    Fleet fleet = allFleets[i];
    if (fleet.DestinationPlanet() != IDPlanet || fleet.TurnsRemaining() != turnNumber)
      continue;
    if (fleet.Owner() == IDPlayer)
      TotalIncomingFleets += fleet.NumShips();
    // else if (IDPlayer != 0)
    //   TotalIncomingFleets -= fleet.NumShips();
    else
      TotalIncomingFleets -= fleet.NumShips();
  }
  return TotalIncomingFleets;
}

int ComputePerimeter(const PlanetWars& pw) {
  int n = pw.NumPlanets(),N = 0,perimeter=0;
  double d = 0.0;
  for (int i=0;i<n-1;i++) {
    for (int j=i+1;j<n;j++) {
      d += float(pw.Distance(i,j));
      N += 1;

    }
  }

  d/= float(N);
  perimeter = int(d/1.5);
  return perimeter;
}

int GetGrowth(const PlanetWars& pw, Planet planet, int BeforeTurn) {
  int TotalGrowth = 0;
  if(planet.Owner()==0)return 0;
  else if(planet.Owner()==1)return planet.GrowthRate() * BeforeTurn;
  else return -1 * planet.GrowthRate() * BeforeTurn;
  

   
}

vector<Planet> enemyPlanetVulnerability(const PlanetWars& pw, Planet enemyPlanet, std::vector<int> MyPlanetsSpareShips) {
  Attack = false;
  vector<Planet> AttackPlanets;
  //perimeter = 7;
  vector<Planet> EnemyPlanet = pw.EnemyPlanets();

  int t;
  for(t=0;t<enemy_futureplanets.size();t++){if(enemy_futureplanets[t].id==enemyPlanet.PlanetID())break;}
  int totalShips = enemy_futureplanets[t].pred[min(NUMBER_OF_PRED-1,perimeter)].second;

  for (int i=0;i<EnemyPlanet.size();i++) {
    Planet NonNeutralPlanet = EnemyPlanet[i];
    int distance = pw.Distance(NonNeutralPlanet.PlanetID(),enemyPlanet.PlanetID());
    if (distance>perimeter)
      continue;
    int TotalSendableShips = MyNumShips[NonNeutralPlanet.PlanetID()] + NonNeutralPlanet.GrowthRate()*(perimeter-distance);
    totalShips -= TotalSendableShips;
  }

  vector<Planet> MyPlanet = pw.MyPlanets();
  for (int i=0;i<MyPlanet.size();i++) {
    Planet NonNeutralPlanet = MyPlanet[i];
    int distance = pw.Distance(NonNeutralPlanet.PlanetID(),enemyPlanet.PlanetID());
    if (distance>perimeter)
      continue;
    int TotalSendableShips = MyPlanetsSpareShips[NonNeutralPlanet.PlanetID()] + NonNeutralPlanet.GrowthRate()*(perimeter-distance);
    totalShips += TotalSendableShips;
    AttackPlanets.push_back(NonNeutralPlanet);
    if (totalShips>0)
      break;
  }  
  if (totalShips>0)Attack = true;
  return AttackPlanets;
}


struct compare{
  bool operator () (const int a, const int b) const
  {
    return a < b;
  }
};

struct compareGR{
  bool operator () (const int a, const int b) const
  {
    return idPlanet[a].GrowthRate() > idPlanet[b].GrowthRate();
  }
};

vector <map <int, int, compare> > my_fleet;

void fleetAggr(const PlanetWars& pw)
{
    
  map <int, int, compare> m;
  for(int k = 0; k <pw.NumPlanets(); k++)my_fleet.push_back(m);
    vector<Fleet> fleets = pw.Fleets();
    for(int i = 0; i <fleets.size(); i++)
    {
      int id = fleets[i].DestinationPlanet(); 
      int turn = fleets[i].TurnsRemaining();
      int num;
      if(fleets[i].Owner() == 1) num = fleets[i].NumShips();
      else num = -fleets[i].NumShips();
      if(my_fleet[id].find(turn) == my_fleet[id].end())
      {
       my_fleet[id][turn] = num;
      }
      else my_fleet[id][turn] = my_fleet[id][turn] + num;
    }
} 

//given owner, gives nearest distance and planet to Planet p
Planet getClosestDist(const PlanetWars& pw,Planet p,int &dist,int pid){
    vector<Planet> checkplanets;
    Planet closest=p; //have to initalize
    dist=99999;
    if(pid==1)checkplanets=pw.MyPlanets();
    else if(pid==2)checkplanets=pw.EnemyPlanets();
    else {checkplanets=pw.NeutralPlanets();}

    for(int i=0;i<checkplanets.size();i++){
      int d=pw.Distance(p.PlanetID(),checkplanets[i].PlanetID());
      if(std::find(neutral_notto_attack.begin(),neutral_notto_attack.end(),p.PlanetID()) != neutral_notto_attack.end())
        continue;
      if(d<dist){dist=d;closest=checkplanets[i];}
    }
    return closest;

}

vector<Planet> getClose(const PlanetWars& pw,Planet p, int dist,int pid){    //lte dist
      vector<Planet> checkplanets,res;
      if(pid==1)checkplanets=pw.MyPlanets();
      else if(pid==2)checkplanets=pw.EnemyPlanets();
      else {checkplanets=pw.NeutralPlanets();}

      for(int i=0;i<checkplanets.size();i++){
        int d=pw.Distance(p.PlanetID(),checkplanets[i].PlanetID());
        if(d<=dist)res.push_back(checkplanets[i]);
      }
      return res;
}



void Predictions(const PlanetWars& pw){
  vector<Planet> my_planets=pw.MyPlanets();
  vector<Planet> enemy_planets=pw.EnemyPlanets();
  vector<Planet> neutral_planets=pw.NeutralPlanets();


  for(int i=0;i<my_planets.size();i++){
    vpii prediction;
    Planet p= my_planets[i];
    int id=p.PlanetID();
    int owner=p.Owner();
    int count=p.NumShips();
    for(int j=1;j<=NUMBER_OF_PRED;j++){
        int turn=j,nships=0;
        if(my_fleet[id].find(turn)!=my_fleet[id].end())nships=my_fleet[id][turn];
        count+=nships;
        if(count < 0)owner=2;
        else if(count>0)owner=1;
        else owner=0;
        
        pair<int,int> pr(owner,count);
        prediction.push_back(pr); 

        if(owner==1)count+=p.GrowthRate();
        else if(owner==2)count-=p.GrowthRate();   
    }
    my_futureplanets[i].id=id;
    my_futureplanets[i].pred=prediction;
    my_futureplanets[i].owner=owner;

    if(owner==2||count==0)my_urgent.push_back(id);          
  }
  


  for(int i=0;i<neutral_planets.size();i++){
    vpii prediction;
    Planet p= neutral_planets[i];
    int id=p.PlanetID();
    int owner=p.Owner();
    int count=-p.NumShips();
    for(int j=1;j<=NUMBER_OF_PRED;j++){
        int turn=j,nships=0;
        if(my_fleet[id].find(turn)!=my_fleet[id].end())nships=my_fleet[id][turn];
        if(owner==0){
          if(nships<0 && abs(nships)>abs(count)){count=-(abs(nships)-abs(count));owner=2;}
          else if(nships>0){count+=nships;owner=1; }
        }
        else{
          count+=nships;
          if(count<0)owner=2;
          else if(count>0)owner=1;
          else owner=0;
        }
        pair<int,int> pr(owner,count);
        prediction.push_back(pr); 

        if(owner==1)count+=p.GrowthRate();
        else if(owner==2)count-=p.GrowthRate();           
    }
    neutral_futureplanets[i].id=id;
    neutral_futureplanets[i].pred=prediction;
    neutral_futureplanets[i].owner=owner;
    //pii pr(id);
    if(owner==2)neutral_urgent.push_back(id);
    else if(owner==1)neutral_notto_attack.push_back(p.PlanetID());

  }



  for(int i=0;i<enemy_planets.size();i++){
    vpii prediction;
    Planet p= enemy_planets[i];
    int id=p.PlanetID();
    int owner=p.Owner();
    int count=-p.NumShips();
    for(int j=1;j<=NUMBER_OF_PRED;j++){
        int turn=j,nships=0;
        if(my_fleet[id].find(turn)!=my_fleet[id].end())nships=my_fleet[id][turn];
        count+=nships;
        if(count<0)owner=2;
        else if(count>0)owner=1;
        else owner=0;
        
        pair<int,int> pr(owner,count);
        prediction.push_back(pr); 

        if(owner==1)count+=p.GrowthRate();
        else if(owner==2)count-=p.GrowthRate();
        
    }
    enemy_futureplanets[i].id=id;
    enemy_futureplanets[i].pred=prediction;
    enemy_futureplanets[i].owner=owner;

    if(owner==2 || count==0)enemy_urgent.push_back(id);          
  }
 
         
}

//idPlanet
int safety=1;
  //sort as per priority order

void Save_MyUrgent(const PlanetWars& pw){     //if myplanet==
      for(int i=0;i<my_urgent.size();i++){
        //how many turns after we lose the planet ultimately
        int id=my_urgent[i]; //turn=my_urgent[i].second;
        Planet p=idPlanet[id];

        int j;
        for(j=0;j<my_futureplanets.size();j++){if(id==my_futureplanets[j].id)break;}
        vpii pred=my_futureplanets[j].pred;
        
        //first turn after which enemy retains the planet
        int turn=NUMBER_OF_PRED-1;
        while( turn>=0 && pred[turn].first!=1)turn--;
        turn++;

        int ns=my_futureplanets[j].pred[NUMBER_OF_PRED-1].second; 
        int shipsneeded=safety*ns+1;
        vector<Planet> close=getClose(pw,p,turn,1);
        for(int j=0;j<close.size();j++){
          if(shipsneeded<=0)break;
          Planet helper=close[j];
          int num_ships=0;
          if(MyPlanetsSpareShips[helper.PlanetID()]<MyNumShips[helper.PlanetID()]){
            num_ships=min(MyPlanetsSpareShips[helper.PlanetID()],shipsneeded);
            pw.IssueOrder(helper.PlanetID(),p.PlanetID(),num_ships);
            MyNumShips[helper.PlanetID()] -= num_ships;
           // out<<TURN<<" "<<helper.PlanetID()<<" "<<p.PlanetID()<<" "<<num_ships<<endl;
            shipsneeded-=num_ships;
            MyPlanetsSpareShips[helper.PlanetID()] -= num_ships;
            // helper.RemoveShips(num_ships);
          }
        }

        if(close.size()==0){
          //can make turn upto=no of predictions
        }
    }
}

void Save_NeutralUrgent(const PlanetWars& pw){
  //sort as per priority order
  for(int i=0;i<neutral_urgent.size();i++){
        //how many turns after we lose the planet ultimately
        int id=neutral_urgent[i];
        int j;
        for(j=0;j<neutral_futureplanets.size();j++){if(id==neutral_futureplanets[j].id)break;}
        vpii pred=neutral_futureplanets[j].pred;
        
        //first turn after which enemy retains the planet
        int turn=NUMBER_OF_PRED-1;
        while( turn>=0 && pred[turn].first!=1)turn--;
        turn++; 
        Planet p=idPlanet[id];
        int d1=99999,d2=99999;
        Planet helper=getClosestDist(pw,p,d1,1);
        Planet attacker=getClosestDist(pw,p,d2,2);
        bool cant_attack=false;
        int enemy_Planet = MyNumShips[attacker.PlanetID()] + attacker.GrowthRate()*(d1-d2);
        // if(d2<d1)continue;  //enemy is closer no point attacking
        //if I am closer
        //check helper is not urgent  
        for(int k=0;k<my_urgent.size();k++){if(my_urgent[i]==helper.PlanetID())cant_attack=true;}
        if(cant_attack)break;      
        int shipsneeded=abs(pred[turn].second)+p.GrowthRate()+1;  //1 extra to win, growth rate added    
        if(my_fleet[id].find(turn+1)!=my_fleet[id].end())shipsneeded+=my_fleet[id][turn+1];

        if(d1==turn+1){   //shud it be exactly equal
          if(MyPlanetsSpareShips[helper.PlanetID()]<(shipsneeded+enemy_Planet) && d2<d1)
            continue;
          else if(shipsneeded>0){
            pw.IssueOrder(helper.PlanetID(),id,shipsneeded);
            MyNumShips[helper.PlanetID()] -= shipsneeded;
            MyPlanetsSpareShips[helper.PlanetID()] -= shipsneeded;
          
            // helper.RemoveShips(shipsneeded);

            neutral_notto_attack.push_back(p.PlanetID());
          }
        }
  }

}
void Target_Neutral(const PlanetWars& pw) {     //chnge no of iterations
  int source = -1;
  Planet p_source = pw.MyPlanets()[0];
  int source_num_ships;
  double source_score;

  //Find Strongst Planet
  for (int i=0;i<10;i++) {      //check it
    
    source_score = -999999.0;
    source_num_ships = 0;
    std::vector<Planet> my_planets = pw.MyPlanets();
    for (int j=0;j<my_planets.size();j++) {
    Planet p = my_planets[j];
      if (std::find(my_urgent.begin(),my_urgent.end(),p.PlanetID()) != my_urgent.end())
        continue;
      float score = float(MyNumShips[p.PlanetID()]);
      if (score > source_score) {
        source_score = score;
        source = p.PlanetID();
        p_source = p;
      }
    }
  
    if(source<0)continue;
    int dest = -1;
    double dest_score = -999999.0;
    std::vector<Planet> not_my_planets = pw.NeutralPlanets();

    for (int i=0;i<not_my_planets.size();i++) {
      Planet p = not_my_planets[i];
      int distance2;
      Planet idPlanet2 = getClosestDist(pw,p,distance2,2);
      if ((std::find(neutral_notto_attack.begin(),neutral_notto_attack.end(),p.PlanetID()) != neutral_notto_attack.end()) )continue; 
      // if (ShipsToPlanetBeforeTurn(pw,p.PlanetID(),NUMBER_OF_PRED,1) > 200)
      //   continue;
      int distance1 = pw.Distance(source,p.PlanetID());

      int enemy_ships=idPlanet2.NumShips()+idPlanet2.GrowthRate()*(distance1-distance2);  //change this
      if (distance1 >= distance2 && MyPlanetsSpareShips[source]<=enemy_ships)
        continue;

      float score = (1.0+p.GrowthRate()) / (1.0 +pw.Distance(source,p.PlanetID())); //change this
      if (score > dest_score) {
        dest_score = score;
        dest = p.PlanetID();
      }
    }
    if (dest_score > 0.0 && source != -1 && MyPlanetsSpareShips[source] > 1 && MyNumShips[source] > 1 && dest != -1  && (std::find(my_urgent.begin(),my_urgent.end(),source) == my_urgent.end())) {
      int num_ships = MyNumShips[dest] + 1;
      //out<<dest<<" "<<endl;
      if (num_ships > 0 && MyNumShips[source] > num_ships && MyPlanetsSpareShips[source] > num_ships) {
        //write
        //out<<TURN<<" "<<source<<" "<<dest<<" "<<num_ships<<" ";


        pw.IssueOrder(source,dest,num_ships);
        neutral_notto_attack.push_back(dest);
        MyPlanetsSpareShips[source] -= num_ships;
       // out<<MyNumShips[source]<<" ";
        // p_source.RemoveShips(num_ships);
        MyNumShips[source]-=num_ships;

       // out<<MyNumShips[source]<<" ";



      }
      // else
      //   break;
    }
  }
}

void Attack_Enemy(const PlanetWars& pw, vector <int> enemyFrontier)
{
    //sort it change this
    sort(enemyFrontier.begin(), enemyFrontier.end(),compareGR());
		for(int i = 0; i <enemyFrontier.size(); i++)
		{ 
      out<<TURN<<" "<<enemyFrontier[i]<<endl;
			vector <Planet> attacker = enemyPlanetVulnerability(pw, pw.GetPlanet(enemyFrontier[i]), MyPlanetsSpareShips);
			if(!Attack)continue;
			for(int j = 0; j <attacker.size(); j++)
			{
        int distance = pw.Distance(enemyFrontier[i],attacker[j].PlanetID());
        int num;
        int t;
        for(t=0;t<enemy_futureplanets.size();t++){if(enemy_futureplanets[t].id==enemyFrontier[i])break;}
        num = abs(enemy_futureplanets[t].pred[NUMBER_OF_PRED-1].second);   //add growthrate if distance>20
        if (num>MyPlanetsSpareShips[attacker[j].PlanetID()] && flag)
          continue;                                                       //don't check if flag>notmyplanets.size()/some factor
				if(find(my_urgent.begin(),my_urgent.end(),attacker[j].PlanetID()) != my_urgent.end())continue;
				int num_ships = min(MyPlanetsSpareShips[attacker[j].PlanetID()], MyNumShips[attacker[j].PlanetID()] -1);
				if(num_ships <= 0)continue;
				pw.IssueOrder(attacker[j].PlanetID(), enemyFrontier[i], num_ships);
        out<<TURN<<" "<<attacker[j].PlanetID()<<" "<<enemyFrontier[i]<<" "<<num_ships<<endl;
        MyNumShips[attacker[j].PlanetID()] -= num_ships;
				MyPlanetsSpareShips[attacker[j].PlanetID()] -= num_ships;
				// attacker[j].RemoveShips(num_ships);
			}
		}
	
}

void SpareShips(const PlanetWars& pw)
{
	std::vector<Planet> my_planets = pw.MyPlanets();
      for (int k=0;k<my_planets.size();k++) {
        int spareShips = 999999;
        int id=my_planets[k].PlanetID();
        int t;
        for(t=0;t<my_futureplanets.size();t++){if(my_futureplanets[t].id==id)break;}
        vpii future_planets = my_futureplanets[t].pred;
        for (int i = 0; i <future_planets.size() ; ++i)
        {
          if (future_planets[i].second<spareShips)
          {
            spareShips = future_planets[i].second;
          }
        }
        MyPlanetsSpareShips[id] = max(0,spareShips);
      }
}
void DoTurn(const PlanetWars& pw) {
      //intialization
      my_fleet.clear();
      TURN++;

      if(pw.MyPlanets().size() < pw.NotMyPlanets().size()/4)
        flag=true;
      else
        flag=false;
      initialize(pw);
      fleetAggr(pw);
	    perimeter = ComputePerimeter(pw);
      perimeter=20;
      my_futureplanets.resize(pw.MyPlanets().size());
      neutral_futureplanets.resize(pw.NeutralPlanets().size());
      enemy_futureplanets.resize(pw.EnemyPlanets().size());

      my_urgent.clear();
      enemy_urgent.clear();
      neutral_urgent.clear();
      neutral_notto_attack.clear();
      MyPlanetsSpareShips.clear();
      MyPlanetsSpareShips.resize(pw.Planets().size());
      MyNumShips.clear();
      MyNumShips.resize(pw.Planets().size());
      vector<Planet> planets=pw.Planets();

      for(int i=0;i<planets.size();i++){
        MyNumShips[i]=planets[i].NumShips();
      }

      
      Predictions(pw);
      SpareShips(pw);

      out<<" My urgent  ";
      for(int i=0;i<my_urgent.size();i++)out<<my_urgent[i]<<" ";
        out<<endl;

      out<<"neutral urgent  ";
      for(int i=0;i<neutral_urgent.size();i++)out<<neutral_urgent[i]<<" ";
        out<<endl;

      out<<"enemy urgent  ";
      for(int i=0;i<enemy_urgent.size();i++)out<<enemy_urgent[i]<<" ";
        out<<endl;

      if(pw.MyPlanets().size()>1){Save_MyUrgent(pw);} 
      Save_NeutralUrgent(pw);

      Target_Neutral(pw);
      Attack_Enemy(pw, enemy_urgent);      
}


// This is just the main game loop that takes care of communicating with the
// game engine for you. You don't have to understand or change the code below.
int main(int argc, char *argv[]) {
  out.open("output.txt");
  string current_line;
  string map_data;
  while (true) {
    int c = cin.get();
    current_line += (char)c;
    if (c == '\n') {
      if (current_line.length() >= 2 && current_line.substr(0, 2) == "go") {
        PlanetWars pw(map_data);
        map_data = "";
        DoTurn(pw);
	pw.FinishTurn();
      } else {
        map_data += current_line;
      }
      current_line = "";
    }
  }
  out.close();
  return 0;
}
