#include <string>
#include <iostream>
#include <sstream> 
#include<map>

using namespace std;
string findkey(multimap<string,string>&map1,string value){
    multimap<string,string>::iterator it;
    for( it=map1.begin();it!=map1.end();it++){
        if((it->second).compare(value) == 0 ){
            return it->first;
        }
    }
    return "\0";
}
int main()
{
  int days;
  cin>>days;
  int clusters;
  cin>>clusters;
  string names[clusters];
  map<string,int>name_mapper;
  int daylimit[clusters],tankcapacity[clusters],currwater[clusters];
  for(int i=0;i<clusters;i++){
      cin>>names[i]>>daylimit[i]>>tankcapacity[i];
      currwater[i]=0;
      name_mapper[names[i]]=i+1;
  }
  int links,j;
  cin>>links;
  string temp;
  multimap<string,string>map1;
  for(int i=0;i<links;i++){
      cin>>temp;
      for(j=0;j<temp.length();j++){
          if(temp[j]=='_')
          break;
      }
      map1.insert(pair<string,string>(temp.substr(0,j),temp.substr(j+1,temp.length()-j-1)));
  }

  int water_consumed=0;
  int flag=1;
  int visited[clusters];
  while(days--){
      for(int i=0;i<clusters;i++){
          visited[i]=false;
      }
      for(int i=0;i<clusters;i++){
          if(visited[i])
          continue;
          
          if(daylimit[i]>currwater[i]){
              visited[i]=true;
              string temp=names[i];
              water_consumed += tankcapacity[i];
              currwater[i]=tankcapacity[i];
              while(1){
                  temp=findkey(map1,temp);
                  if(temp=="F")
                  break;
                  if(temp=="\0")
                  {
                      flag=0;
                      break;
                  }
                  if(!visited[name_mapper[temp]-1])
                  {
                      water_consumed += tankcapacity[name_mapper[temp]-1];
                  }
                  
                  currwater[name_mapper[temp]-1]=tankcapacity[name_mapper[temp]-1];
              }
          }
          
          
      }
      
          for(int i=0;i<clusters;i++)
          currwater[i] -= daylimit[i];
  }

  cout<<water_consumed;
  return 0;
}
