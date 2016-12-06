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
    MyClass helper("000","yyy",0,0);
    MyClass wodner1("111","thief",999,56.4);
    MyClass wodner2("222","fly",88,54.2);
    MyClass wodner3("333","universe",5,45.9);
    MyClass wodner4("444","keybord",34,99.9);

    listClassObject.push_back(wodner1);
    listClassObject.push_back(wodner2);
    listClassObject.push_back(wodner3);
    listClassObject.push_back(wodner4);

    try {
       // mapper.createTbl(wodner1);
      //  mapper.createTbl(wodner1,1,"number");

     //   mapper.select(helper,Exp("name,id")).toVector();
        mapper.insertRange(listClassObject);
        mapper.insert(wodner1);
     //   mapper.select(helper).toVector();
       // mapper.update(wodner1);
       // mapper.updateRange(listClassObject);
        std::vector<int> a{1,3,5},b{2,4,6};
        std::vector<std::string> c{"thief","woder","hehe"};

       // mapper.dropTbl(helper);
    //    mapper.deleteRow(wodner1);

      // ORM_MYSQL_OP::OnePiece mi("int","44");
      //  int gf=0;
     //   gf=mi;
     //   ORM_MYSQL_OP::OnePiece ma("string","haha");
      //  std::string sd=ma;
      //  int ss=ma;
       // std::cout<<gf<<sd<<ss;
        int counter=mapper.query(helper).where(Exp("level")<88).count();
        std::cout<<"count  "<<counter<<"\n";
        //auto g=mapper.select(wodner1,Exp("name,id,level")).toVector(); select(wodner1,Exp("name,id,level,score"))
        auto g=mapper.query(helper).where(Exp("level")<90).limit(2).offset(0).toVector();
        //auto cr=mapper.select(helper).query().where(Exp("level")<78).toVector();

        for(auto h:g)
        {
            MyClass lol(h[0],h[1],h[2],h[3]);
            lol.printItself();
        }
        std::cout<<mapper.query(helper).where(Exp("level")<80).count();

       // mapper.dropTbl(wodner1);
    }catch (const std::exception &e)
    {
        std::cout<<e.what();

    }

    return 0;
}