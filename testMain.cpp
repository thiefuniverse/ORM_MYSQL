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
        mapper.select(wodner1,Exp("name,id")).toVector();
        mapper.insertRange(listClassObject);

    }catch (const std::exception &e)
    {
        std::cout<<e.what();

    }
   // ORM_MYSQL_OP::FieldManager a;
//    std::cout<<a.getRealVal(std::string("567"),std::string("int"));

    return 0;
}