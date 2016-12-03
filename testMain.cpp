//
// Created by thief on 16-11-25.
//

#include "orm_mysql.h"
#include <iostream>
#include <string>

using namespace ORM_MYSQL_THIEF;
class MyClass
{
ORMAP_MYSQL(serverDB,id,name,level,score)
public:
    MyClass(std::string idr,std::string namer,int levels,float score)
            :id(idr),name(namer),level(levels),score(score)
    {}

    std::string id;
    std::string name;
    int level;
    float score;

    void printItself()
    {
        std::cout<<id<<"   "<<name<<"  "<<level<<"  "<<score<<"  "<<std::endl;
    }
};


int main()
{
    ORMapper mapper("localhost","thief","ssopq","test",0);
    std::vector<MyClass> listClassObject;
    MyClass wodner1("111","thief",999,56.4);
    MyClass wodner2("222","fly",88,54.2);
    MyClass wodner3("333","universe",5,45.9);
    listClassObject.push_back(wodner1);
    listClassObject.push_back(wodner2);
    listClassObject.push_back(wodner3);
    //mapper.dropTbl(wodner1);
    try {
      // mapper.createTbl(wodner1);
//        mapper.select(wodner1,Exp("name,id")).toVector();
  //      mapper.insertRange(listClassObject);
       // mapper.select(wodner1).toVector();

        std::vector<int> a{1,3,5},b{2,4,6};
        std::vector<std::string> c{"thief","woder","hehe"};
      // ORM_MYSQL_OP::OnePiece mi("int","44");
      //  int gf=0;
     //   gf=mi;
     //   ORM_MYSQL_OP::OnePiece ma("string","haha");
      //  std::string sd=ma;
      //  int ss=ma;
       // std::cout<<gf<<sd<<ss;

        //auto g=mapper.select(wodner1,Exp("name,id,level")).toVector(); select(wodner1,Exp("name,id,level,score"))
        auto g=mapper.query(wodner1).where(Exp("level")<90).toVector();
        for(auto h:g)
        {
            MyClass lol(h[0],h[1],h[2],h[3]);
            lol.printItself();
//            std::string name=h[0];
//            std::string id=h[1];
//            int level=h[2];
//            float score=h[3];
//            std::cout<<id<<" "<<name<<" "<<level<<" "<<score<<"\n";
        }
    }catch (const std::exception &e)
    {
        std::cout<<e.what();

    }

    return 0;
}