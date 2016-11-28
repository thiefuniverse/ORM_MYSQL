//
// Created by thief on 16-11-25.
//

#include "orm_mysql.h"
#include <iostream>
#include <string>

using namespace ORM_MYSQL_THIEF;
class PokemonServer
{
ORMAP_MYSQL(serverDB,id,name,level)

public:


    PokemonServer(std::string idr,std::string namer,int levels)
            :id(idr),name(namer),level(levels)
    {

    }
    std::string serverDB;
    std::string id;
    std::string name;
    int level;


};

int main()
{
    ORMapper thief("localhost","thief","ssopq","test",0);
   // thief.createTbl(PokemonServer ("f","y",888));
    std::vector<PokemonServer> listPokemon;
    PokemonServer wodner1("efil","s1",12);
    PokemonServer wodner2("ec","s2",23);
    PokemonServer wodner3("ed","s3",45);
    listPokemon.push_back(wodner1);
    listPokemon.push_back(wodner2);
    listPokemon.push_back(wodner3);
    thief.updateRange(listPokemon);

  //  thief.insertRange(listPokemon);
   // thief.dropTbl(PokemonServer());
   // thief.initField();
   // PokemonServer t;

    decltype(77) hi;
}