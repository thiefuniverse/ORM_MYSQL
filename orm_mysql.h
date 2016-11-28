//
// Created by thief on 16-11-25.
//

#ifndef TESTCONNECT_ORM_MYSQL_H
#define TESTCONNECT_ORM_MYSQL_H


#include <mysql.h>
#include <string>
#include <stdexcept>
#include <ext/hash_map>
#include <algorithm>
#include <tuple>
#include <iostream>
#include <sstream>
#include <typeinfo>

#define ORMAP_MYSQL(_CLASS_NAME_,...) \
private:  \
friend class ORM_MYSQL_THIEF::ORMapper; \
template<typename Visitor,typename Fn> \
void _Transfer(const Visitor &visitor,Fn fn) \
{                                            \
      visitor.Visit(fn,__VA_ARGS__);         \
       \
};                                           \
public:                                      \
constexpr static const char* _className=#_CLASS_NAME_; \
constexpr static const char* _fieldName=#__VA_ARGS__;\
private:                                        \

//   fn(__VA_ARGS_);

namespace ORM_MYSQL_OP
{
    class SQLConnector
    {
    public:
        SQLConnector (const std::string &host, const std::string &user,
                const std::string &password,const std::string &db,int port)
        {
            connector=mysql_init(NULL);
            if (connector==NULL)
            {
                throw std::runtime_error(
                  std::string("Can't init mysql connection.\n")
                  + std::string(mysql_error(connector))
                );
            }

            //todo maybe args socket and clientflag can be configured
            if(mysql_real_connect(connector,host.c_str(),user.c_str(),
            password.c_str(),db.c_str(),port,NULL,0)==NULL)
            {
                throw std::runtime_error(
                        std::string("Can't connect mysql database.\n")
                        + std::string(mysql_error(connector))
                );
            }

        }

        ~SQLConnector()
        {
            mysql_close(connector);
        }

        void execute(const std::string & cmd)
        {
            mysql_query(connector,cmd.c_str());
            std::string errorInfo=std::string(mysql_error(connector));
            if (!errorInfo.empty())
            {
                throw std::runtime_error(
                        std::string("error for execute "+cmd+".\n")
                        + std::string(mysql_error(connector))
                );
            }
        }

    private:
        MYSQL *connector;
    };


    class FnVisitor
    {
    public:
        template <typename Fn, typename... Args>
        inline void Visit (Fn fn, Args & ... args) const
        {
            _Visit (fn, args...);
        }

    protected:
        template <typename Fn, typename T, typename... Args>
        inline void _Visit (Fn fn, T &property, Args & ... args) const
        {
            if (_Visit (fn, property))
                _Visit (fn, args...);
        }

        template <typename Fn, typename T>
        inline bool _Visit (Fn fn, T &property) const
        {
            return fn (property);
        }
    };


    class FieldManager
    {
    public:
        // extract fieldName and fieldType from primitive _VA_ARGS_

        template <typename T>
        static const std::vector<std::string> extractField()
        {
            std::vector<std::string> fieldName;
            std::string rawStr(T::_fieldName);

            std::string tempName="";
            int rawLen=rawStr.length();
            for(int i=0;i<rawLen;++i)
            {
                if (rawStr[i]!=',')
                {
                    tempName+=rawStr[i];
                }
                else
                {
                    fieldName.push_back(tempName);
                    tempName.clear();
                }
            }
            fieldName.push_back(tempName);
            return std::move(fieldName);
        }

        // Helper - Get TypeString
        template <typename T>
        static const char *TypeString (T &t)
        {
            constexpr const char *typeStr =
                    (std::is_integral<T>::value &&
                     !std::is_same<std::remove_cv_t<T>, char>::value &&
                     !std::is_same<std::remove_cv_t<T>, wchar_t>::value &&
                     !std::is_same<std::remove_cv_t<T>, char16_t>::value &&
                     !std::is_same<std::remove_cv_t<T>, char32_t>::value &&
                     !std::is_same<std::remove_cv_t<T>, unsigned char>::value)
                    ? "int"
                    : (std::is_floating_point<T>::value)
                      ? "float"
                      : (std::is_same<std::remove_cv_t<T>, std::string>::value)
                        ? "varchar(100)"
                        : nullptr;
//            std::cout<< (typeid(1)== typeid(t))<<"\n";
//            std::cout<< (typeid(1.3)== typeid(t))<<"\n";
//            std::cout<< (typeid("thief")== typeid(t))<<"\n";
//            std::cout<<typeid(&t).name()<<"\n";
            static_assert (
                    typeStr != nullptr,
                    "Only Support Integral, Floating Point and std::string :-)");

            return typeStr;
        }



    };

    // Helper - Serialize
    template <typename T>
    inline std::ostream &SerializeValue (std::ostream &os,
                                         const T &value)
    {
        return os << value;
    }

    template <>
    inline std::ostream &SerializeValue <std::string>
            (std::ostream &os, const std::string &value)
    {
        return os << "'" << value << "'";
    }

    // Helper - Deserialize
    template <typename T>
    inline void DeserializeValue (T &property, std::string value)
    {
        std::stringstream ostr (value);
        ostr >> property;
    }

    template <>
    inline void DeserializeValue <std::string>
            (std::string &property, std::string value)
    {
        property = std::move (value);
    }
}

namespace ORM_MYSQL_THIEF
{
    class ORMapper
    {
    public:

        ORMapper(const std::string &host, const std::string &user,
                  const std::string &password,const std::string &db,int port)
        :_connector(host,user,password,db,port)
        {
           // _fieldManager=new ORM_MYSQL_OP::FieldManager();

        }

        /************************************************************************
        different operations on sql
        ************************************************************************/
        // create table
        template <typename T>
        bool createTbl(const T& classObject)
        {
            // get fieldName and fieldType
            _fieldName=ORM_MYSQL_OP::FieldManager::extractField<T> ();
            std::vector<std::string> _fieldType(_fieldName.size());
            unsigned int index=0;

            classObject._Transfer(ORM_MYSQL_OP::FnVisitor(),
                                  [&_fieldType,&index](auto &val)
                                  {
                                      _fieldType[index++]=ORM_MYSQL_OP::FieldManager::TypeString(val);
                                      return true;
                                  });

            std::string createStr="create table "+std::string(T::_className)+" ";

            std::string createField="";
            for(unsigned int i=0;i<_fieldType.size();++i)
            {
                createField+=_fieldName[i]+" "+std::string(_fieldType[i])+",";
            }
            createField+=" primary key ("+_fieldName[0]+" )";
            //todo execute create
            _connector.execute(createStr+" ( "+createField+" )");
        }

        template <typename T>
        void dropTbl(const T &className)
        {
            _connector.execute("drop table "+std::string(T::_className));
        }

        template <typename T>
        void insert(T& classObject)
        {
            std::string insertStr="insert into "+std::string(T::_className)+
            " ("+std::string(T::_fieldName)+") values ";
            unsigned int index=ORM_MYSQL_OP::FieldManager::extractField<T>() .size();
            std::ostringstream valuesStr;
            classObject._Transfer(ORM_MYSQL_OP::FnVisitor(),
                                  [&valuesStr,&index](auto &value)
            {
                if (index>1)
                {
                    ORM_MYSQL_OP::SerializeValue(valuesStr,value)<<",";
                }
                else
                {
                    ORM_MYSQL_OP::SerializeValue(valuesStr,value);
                }
                index--;
                return true;
            });

            _connector.execute(insertStr+" ("+valuesStr.str()+")");

        }

        template <typename nT>
        void insertRange(nT &classObjects)
        {
            using T=typename nT::value_type;
            std::string insertStr="insert into "+std::string(T::_className)+
                                  " ("+std::string(T::_fieldName)+") values ";
            _fieldName=ORM_MYSQL_OP::FieldManager::extractField<T> ();
            for(auto classObject:classObjects)
            {
                unsigned int index=_fieldName.size();
                std::ostringstream valuesStr;
                classObject._Transfer(ORM_MYSQL_OP::FnVisitor(),
                                      [&valuesStr,&index](auto &value)
                                      {
                                          if (index>1)
                                          {
                                              ORM_MYSQL_OP::SerializeValue(valuesStr,value)<<",";
                                          }
                                          else
                                          {
                                              ORM_MYSQL_OP::SerializeValue(valuesStr,value);
                                          }
                                          index--;
                                          return true;
                                      });

                _connector.execute(insertStr+" ("+valuesStr.str()+")");
            }

        }

        template <typename T>
        void update(const T& classObject)
        {
            const auto &fieldNames=ORM_MYSQL_OP::FieldManager::extractField<T>();
            std::ostringstream updateStr;
            std::string updateCmd="update "+std::string(T::_className);
            unsigned int index=0;
            std::ostringstream updateKey;
            classObject._Transfer(ORM_MYSQL_OP::FnVisitor(),
                                  [&updateStr,&fieldNames,&updateKey,&index](auto &val)
                                  {
                                      if (index==0)
                                      {
                                          updateKey<<fieldNames[index++]<<"=";
                                          ORM_MYSQL_OP::SerializeValue(updateKey,val);
                                      }
                                      else
                                      {
                                          updateStr<<fieldNames[index++]<<"=";
                                          if (index!=fieldNames.size())
                                          {
                                              ORM_MYSQL_OP::SerializeValue(updateStr,val)<<",";
                                          }
                                          else
                                          {
                                              ORM_MYSQL_OP::SerializeValue(updateStr,val);
                                          }
                                      }
                                      return true;
                                  });
            updateCmd+=" set "+updateStr.str()+" where "+updateKey.str();
            _connector.execute(updateCmd);
        }


        template <typename nT>
        void updateRange(const nT& classObjects)
        {
            using T=typename nT::value_type;

            const auto &fieldNames=ORM_MYSQL_OP::FieldManager::extractField<T>();
            for(auto classObject:classObjects)
            {

                std::ostringstream updateStr;
                std::string updateCmd="update "+std::string(T::_className);
                unsigned int index=0;
                std::ostringstream updateKey;
                classObject._Transfer(ORM_MYSQL_OP::FnVisitor(),
                                      [&updateStr,&fieldNames,&updateKey,&index](auto &val)
                                      {
                                          if (index==0)
                                          {
                                              updateKey<<fieldNames[index++]<<"=";
                                              ORM_MYSQL_OP::SerializeValue(updateKey,val);
                                          }
                                          else
                                          {
                                              updateStr<<fieldNames[index++]<<"=";
                                              if (index!=fieldNames.size())
                                              {
                                                  ORM_MYSQL_OP::SerializeValue(updateStr,val)<<",";
                                              }
                                              else
                                              {
                                                  ORM_MYSQL_OP::SerializeValue(updateStr,val);
                                              }
                                          }
                                          return true;
                                      });
                updateCmd+=" set "+updateStr.str()+" where "+updateKey.str();
                _connector.execute(updateCmd);
            }

        }

        template <typename T>
        void deleteRow(const T& classObject)
        {
            std::string deleteCmd="delete from "+std::string(T::_className);
            std::ostringstream deleteStr;
            classObject._Transfer(ORM_MYSQL_OP::FnVisitor(),
                                  [&deleteStr](auto &val)
                                  {
                                      deleteStr<<ORM_MYSQL_OP::FieldManager::extractField<T>() [0] <<"=";
                                      ORM_MYSQL_OP::SerializeValue(deleteStr,val);
                                      return false;
                                  });
            deleteCmd+=" where "+deleteStr.str();
            _connector.execute(deleteCmd);

        }

        template <typename T>

        /************************************************************************/



       // template <typename T>
       bool initField()
        {

            for (int i = 0; i < _fieldName.size(); ++i) {
                std::cout<<_fieldName[i]<<" ";
            }
            std::cout<<"\n";
            return true;
        }

    private:
        ORM_MYSQL_OP::SQLConnector _connector;
      //  ORM_MYSQL_OP::FieldManager *_fieldManager;
        std::vector<std::string> _fieldName;

    };


    // get expression string
    struct Exp
    {
        std::string realExpr;
        std::vector<std::pair<const void*,std::string>> expr;
        Exp(const std::string &property)
        {
            realExpr+=property;
        }

        template <typename T>
        Exp Field(const std::string& opStr,T value)
        {
            std::ostringstream os;
            ORM_MYSQL_OP::SerializeValue(os,value);
            realExpr+=opStr+os.str();
            realExpr.insert(0,"(");
            realExpr.push_back(')');
            return *this;
        }

        Exp Field(const std::string& opStr,const char* value)
        {
            std::ostringstream os;
            ORM_MYSQL_OP::SerializeValue(os,std::string(value));
            realExpr+=opStr+os.str();
            realExpr.insert(0,"(");
            realExpr.push_back(')');
            return *this;
        }

        template <typename T>
        inline Exp operator == (T value)
        {
            return Field("=",value);
        }
        template <typename T>
        inline Exp operator != (T value)
        {
            return Field("!=",value);
        }
        template <typename T>
        inline Exp operator < (T value)
        {
            return Field("<",value);
        }
        template <typename T>
        inline Exp operator <= (T value)
        {
            return Field("<=",value);
        }
        template <typename T>
        inline Exp operator > (T value)
        {
            return Field(">",value);
        }
        template <typename T>
        inline Exp operator >= (T value)
        {
            return Field(">=",value);
        }

        inline Exp operator || (const Exp& expr)
        {
            realExpr+=" or "+expr.realExpr;
            realExpr.insert(0,"(");
            realExpr.push_back(')');
            return *this;
        }

        inline Exp operator && (const Exp& expr)
        {
            realExpr+=" and "+expr.realExpr;
            realExpr.insert(0,"(");
            realExpr.push_back(')');
            return *this;
        }
    };





}
#endif //TESTCONNECT_ORM_MYSQL_H
