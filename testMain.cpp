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
    PokemonServer wodner1("ef","thief",777);
    PokemonServer wodner2("haha","s2",23);
    PokemonServer wodner3("ethsdfiefdad","563",45);
    listPokemon.push_back(wodner1);
    listPokemon.push_back(wodner2);
    listPokemon.push_back(wodner3);
    //thief.updateRange(listPokemon);
    //thief.insertRange(listPokemon);
    //thief.insert(wodner1);
    try {
      //  thief.update(wodner3);
    }catch (const std::exception &e)
    {
        std::cout<<e.what();
    }

   // std::cout<<(Exp("id")=="thief"|| (Exp("name")<"thief")).realExpr;
//    std::string hi;
//    hi+="'";
//    std::cout<<hi;
  //  thief.insertRange(listPokemon);
   // thief.dropTbl(PokemonServer());
   // thief.initField();
   // PokemonServer t;

}