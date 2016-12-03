#ifndef ORM_MYSQL_H
#define ORM_MYSQL_H

#include <mysql.h>
#include <string>
#include <stdexcept>
#include <ext/hash_map>
#include <algorithm>
#include <tuple>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <tuple>
#include <map>
#include <assert.h>

typedef std::vector<std::vector <std::string>> Sql_Result;

#define ORMAP_MYSQL(_CLASS_NAME_,...)                  \
private:                                               \
friend class ORM_MYSQL_THIEF::ORMapper;                \
template<typename Visitor,typename Fn>                 \
void _Transfer(const Visitor &visitor,Fn fn)           \
{                                                      \
      visitor.Visit(fn,__VA_ARGS__);                   \
                                                       \
};                                                     \
public:                                                \
constexpr static const char* _className=#_CLASS_NAME_; \
constexpr static const char* _fieldName=#__VA_ARGS__;  \
private:                                               \


namespace ORM_MYSQL_OP
{

    class OnePiece
    {
    public:

        //init real value by type string
        OnePiece(const std::string& type,const std::string& val)
        {
            if (type=="int")
            {
                number=1;
                value_int=std::stoi(val);
            }
            else if(type=="float")
            {
                number=2;
                value_float=std::stof(val);
            }
            else if(type=="string")
            {
                number=3;
                value_string=val;
            }
        }

        // overload different type function for assignment
        operator int() const
        {
            return value_int;
        }

        operator float() const
        {
            return value_float;
        }
        operator std::string() const
        {
            return value_string;
        }

        int number;
        int value_int;
        float value_float;
        std::string value_string;

    };


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


        std::vector<std::vector<ORM_MYSQL_OP::OnePiece>> getResultVector(std::vector<std::string>& fields,
                                                                         std::map<std::string,std::string>& nameToType)
        {
            std::vector<std::vector<ORM_MYSQL_OP::OnePiece>> realResult;

            MYSQL_RES *_result=mysql_store_result(connector);
            if (_result!=NULL)
            {
                int num_fields=mysql_num_fields(_result);
                MYSQL_ROW _row;
                while((_row=mysql_fetch_row(_result)))
                {
                    std::vector <ORM_MYSQL_OP::OnePiece> singleRow;
                    for(int i=0;i<num_fields;i++)
                    {
                        OnePiece one(nameToType.find(fields[i])->second,std::string(_row[i]));
                        singleRow.push_back(one);
                     }
                     realResult.push_back(singleRow);

                }
                mysql_free_result(_result);

            }
            return std::move(realResult);
        }


        ~SQLConnector()
        {

            mysql_close(connector);
        }


        void execute(const std::string & cmd)
        {
            mysql_query(connector,cmd.c_str());
            std::string errorInfo=std::string(mysql_error(connector));
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
            static_assert (
                    typeStr != nullptr,
                    "Only Support Integral, Floating Point and std::string :-)");

            return typeStr;
        }



        template <typename T>
        static const char *realTypeString (T &t)
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
                        ? "string"
                        : nullptr;
            static_assert (
                    typeStr != nullptr,
                    "Only Support Integral, Floating Point and std::string :-)");

            return typeStr;
        }

    };


}

namespace ORM_MYSQL_THIEF {
    class Query;
    class Select;
    class Exp;


    // get expression string
    class Exp {
    public:
        std::string realExpr;

        Exp(const std::string &property) {
            realExpr += property;
        }

        Exp(const char *property) {
            realExpr += std::string(property);
        }

        template<typename T>
        Exp Field(const std::string &opStr, T value) {
            std::ostringstream os;
            ORM_MYSQL_OP::SerializeValue(os, value);
            realExpr += opStr + os.str();
            realExpr.insert(0, "(");
            realExpr.push_back(')');
            return *this;
        }

        Exp Field(const std::string &opStr, const char *value) {
            std::ostringstream os;
            ORM_MYSQL_OP::SerializeValue(os, std::string(value));
            realExpr += opStr + os.str();
            realExpr.insert(0, "(");
            realExpr.push_back(')');
            return *this;
        }

        template<typename T>
        inline Exp operator=(T value) {
            return Field("=", std::move(value));
        }

        template<typename T>
        inline Exp operator==(T value) {
            return Field("=", std::move(value));
        }

        template<typename T>
        inline Exp operator!=(T value) {
            return Field("!=", std::move(value));
        }

        template<typename T>
        inline Exp operator<(T value) {
            return Field("<", std::move(value));
        }

        template<typename T>
        inline Exp operator<=(T value) {
            return Field("<=", std::move(value));
        }

        template<typename T>
        inline Exp operator>(T value) {
            return Field(">", std::move(value));
        }

        template<typename T>
        inline Exp operator>=(T value) {
            return Field(">=", std::move(value));
        }

        inline Exp operator||(const Exp &expr) {
            realExpr += " or " + expr.realExpr;
            realExpr.insert(0, "(");
            realExpr.push_back(')');
            return *this;
        }

        inline Exp operator&&(const Exp &expr) {
            realExpr += " and " + expr.realExpr;
            realExpr.insert(0, "(");
            realExpr.push_back(')');
            return *this;
        }
    };

    class Query {
    public:
        Query(std::string &tableName, std::string &selectField, ORM_MYSQL_OP::SQLConnector &connector,
              std::vector<std::string> &fields,
              std::map<std::string, std::string> &nameToType)
                : _queryStr(selectField), _tableName(tableName), _connector(&connector),
                  _nameToType(nameToType) ,_fields(fields)
        {

        }

        Query(std::string &tableName, ORM_MYSQL_OP::SQLConnector &connector, std::vector<std::string> &fieldNames,
              std::map<std::string, std::string> &nameToType)
                : _queryStr("select * from " + tableName + " "), _tableName(tableName), _connector(&connector),
                  _nameToType(nameToType) ,_fields(fieldNames)
        {

        }

        Query &where(const Exp &expr) {
            _sqlwhere = " where " + expr.realExpr;
            _queryStr += _sqlwhere;

            return *this;
        }

        Query &limit(int count1, int count2) {
            _queryStr += " limit " + std::to_string(count1) + "," + std::to_string(count2);
            return *this;
        }

        Query &limit(int count) {
            _queryStr += " limit " + std::to_string(count);
            return *this;
        }

        Query &offset(int count) {
            _queryStr += " offset　" + std::to_string(count);
            return *this;
        }

        void update(const Exp &exp) {
            std::string realExp;
            //delete ( and ) in Exp
            for (auto a:exp.realExpr) {
                if (a != '(' && a != ')') {
                    realExp += a;
                }
            }
            std::string updateStr = "update " + _tableName + " set " + realExp + _sqlwhere;
            _connector->execute(updateStr);
        }

        void del(const Exp &exp) {
            std::string deleteStr = "delete from " + _tableName + _sqlwhere;
            _connector->execute(deleteStr);
        }

        std::vector<std::vector<ORM_MYSQL_OP::OnePiece>> toVector() {
            _connector->execute(_queryStr);
            return std::move(_connector->getResultVector(_fields,_nameToType));
        }

    private:
        std::string _queryStr;
        std::string _tableName;
        std::string _sqlwhere;
        std::vector<std::string> _fields;
        std::map<std::string, std::string> _nameToType;
        ORM_MYSQL_OP::SQLConnector *_connector;
    };

    class Select {
    public:
        Select(const std::string &tableName, Exp &expr, ORM_MYSQL_OP::SQLConnector &connector,
               std::map<std::string, std::string> &nameToType)
                : _selectField("select " + expr.realExpr + " from " + tableName), _connector(&connector),
                  _nameToType(nameToType)
        {
            std::string tempName = "";
            int rawLen = expr.realExpr.length();
            for (int i = 0; i < rawLen; ++i) {
                if (expr.realExpr[i] != ',') {
                    tempName += expr.realExpr[i];
                } else {
                    _fields.push_back(tempName);
                    tempName.clear();
                }
            }
            _fields.push_back(tempName);
        }

        Select(const std::string &tableName, ORM_MYSQL_OP::SQLConnector &connector, std::vector<std::string> &fieldName,
               std::map<std::string, std::string> &nameToType)
                : _selectField("select * from " + tableName), _connector(&connector), _tableName(tableName),
                  _fieldName(fieldName), _nameToType(nameToType) ,_fields(fieldName)
        {

        }

        Query query() {
            Query _query(_tableName, _selectField, *_connector,_fields,_nameToType);
            return std::move(_query);
        }


        std::vector<std::vector<ORM_MYSQL_OP::OnePiece>> toVector() {
            _connector->execute(_selectField);
            return std::move(_connector->getResultVector(_fields, _nameToType));
        }

    private:
        std::vector<std::string> _fields;
        std::vector<std::string> _fieldName;
        std::map<std::string, std::string> _nameToType;
        std::string _selectField;
        std::string _tableName;
        ORM_MYSQL_OP::SQLConnector *_connector;
    };

    class ORMapper {
    public:

        ORMapper(const std::string &host, const std::string &user,
                 const std::string &password, const std::string &db, int port) {
            _connector = new ORM_MYSQL_OP::SQLConnector(host, user, password, db, port);
        }

        ~ORMapper() {}
        /************************************************************************
        different operations on sql
        ************************************************************************/
        // create table
        template<typename T>
        bool createTbl(const T &classObject) {
            // get fieldName and fieldType
            _fieldName = ORM_MYSQL_OP::FieldManager::extractField<T>();
            std::vector<std::string> _fieldType(_fieldName.size());
            unsigned int index = 0;

            classObject._Transfer(ORM_MYSQL_OP::FnVisitor(),
                                  [&_fieldType, &index](auto &val) {
                                      _fieldType[index++] = ORM_MYSQL_OP::FieldManager::TypeString(val);
                                      return true;
                                  });

            std::string createStr = "create table " + std::string(T::_className) + " ";

            std::string createField = "";
            for (unsigned int i = 0; i < _fieldType.size(); ++i) {
                createField += _fieldName[i] + " " + std::string(_fieldType[i]) + ",";
            }
            createField += " primary key (" + _fieldName[0] + " )";
            //todo execute create
            _connector->execute(createStr + " ( " + createField + " )");
        }

        template<typename T>
        void dropTbl(const T &className) {
            _connector->execute("drop table " + std::string(T::_className));
        }

        template<typename T>
        void insert(T &classObject) {
            std::string insertStr = "insert into " + std::string(T::_className) +
                                    " (" + std::string(T::_fieldName) + ") values ";
            unsigned int index = ORM_MYSQL_OP::FieldManager::extractField<T>().size();
            std::ostringstream valuesStr;
            classObject._Transfer(ORM_MYSQL_OP::FnVisitor(),
                                  [&valuesStr, &index](auto &value) {
                                      if (index > 1) {
                                          ORM_MYSQL_OP::SerializeValue(valuesStr, value) << ",";
                                      } else {
                                          ORM_MYSQL_OP::SerializeValue(valuesStr, value);
                                      }
                                      index--;
                                      return true;
                                  });

            _connector->execute(insertStr + " (" + valuesStr.str() + ")");

        }

        template<typename nT>
        void insertRange(nT &classObjects) {
            using T=typename nT::value_type;
            std::string insertStr = "insert into " + std::string(T::_className) +
                                    " (" + std::string(T::_fieldName) + ") values ";
            _fieldName = ORM_MYSQL_OP::FieldManager::extractField<T>();
            for (auto classObject:classObjects) {
                unsigned int index = _fieldName.size();
                std::ostringstream valuesStr;
                classObject._Transfer(ORM_MYSQL_OP::FnVisitor(),
                                      [&valuesStr, &index](auto &value) {
                                          if (index > 1) {
                                              ORM_MYSQL_OP::SerializeValue(valuesStr, value) << ",";
                                          } else {
                                              ORM_MYSQL_OP::SerializeValue(valuesStr, value);
                                          }
                                          index--;
                                          return true;
                                      });

                _connector->execute(insertStr + " (" + valuesStr.str() + ")");
            }

        }

        template<typename T>
        void update(const T &classObject) {
            const auto &fieldNames = ORM_MYSQL_OP::FieldManager::extractField<T>();
            std::ostringstream updateStr;
            std::string updateCmd = "update " + std::string(T::_className);
            unsigned int index = 0;
            std::ostringstream updateKey;
            classObject._Transfer(ORM_MYSQL_OP::FnVisitor(),
                                  [&updateStr, &fieldNames, &updateKey, &index](auto &val) {
                                      if (index == 0) {
                                          updateKey << fieldNames[index++] << "=";
                                          ORM_MYSQL_OP::SerializeValue(updateKey, val);
                                      } else {
                                          updateStr << fieldNames[index++] << "=";
                                          if (index != fieldNames.size()) {
                                              ORM_MYSQL_OP::SerializeValue(updateStr, val) << ",";
                                          } else {
                                              ORM_MYSQL_OP::SerializeValue(updateStr, val);
                                          }
                                      }
                                      return true;
                                  });
            updateCmd += " set " + updateStr.str() + " where " + updateKey.str();
            _connector->execute(updateCmd);
        }


        template<typename nT>
        void updateRange(const nT &classObjects) {
            using T=typename nT::value_type;

            const auto &fieldNames = ORM_MYSQL_OP::FieldManager::extractField<T>();
            for (auto classObject:classObjects) {

                std::ostringstream updateStr;
                std::string updateCmd = "update " + std::string(T::_className);
                unsigned int index = 0;
                std::ostringstream updateKey;
                classObject._Transfer(ORM_MYSQL_OP::FnVisitor(),
                                      [&updateStr, &fieldNames, &updateKey, &index](auto &val) {
                                          if (index == 0) {
                                              updateKey << fieldNames[index++] << "=";
                                              ORM_MYSQL_OP::SerializeValue(updateKey, val);
                                          } else {
                                              updateStr << fieldNames[index++] << "=";
                                              if (index != fieldNames.size()) {
                                                  ORM_MYSQL_OP::SerializeValue(updateStr, val) << ",";
                                              } else {
                                                  ORM_MYSQL_OP::SerializeValue(updateStr, val);
                                              }
                                          }
                                          return true;
                                      });
                updateCmd += " set " + updateStr.str() + " where " + updateKey.str();
                _connector->execute(updateCmd);
            }

        }

        template<typename T>
        void deleteRow(const T &classObject) {
            std::string deleteCmd = "delete from " + std::string(T::_className);
            std::ostringstream deleteStr;
            classObject._Transfer(ORM_MYSQL_OP::FnVisitor(),
                                  [&deleteStr](auto &val) {
                                      deleteStr << ORM_MYSQL_OP::FieldManager::extractField<T>()[0] << "=";
                                      ORM_MYSQL_OP::SerializeValue(deleteStr, val);
                                      return false;
                                  });
            deleteCmd += " where " + deleteStr.str();
            _connector->execute(deleteCmd);

        }

        template<typename T>
        Query query(T &classObject) {
            _fieldName = ORM_MYSQL_OP::FieldManager::extractField<T>();
            std::string tableName = classObject._className;
            std::vector<std::string> fieldName = ORM_MYSQL_OP::FieldManager::extractField<T>();
            std::map<std::string, std::string> nameToType;
            int index = 0;
            classObject._Transfer(ORM_MYSQL_OP::FnVisitor(),
                                  [&nameToType, &index, &fieldName](auto &val) {
                                      nameToType.insert(std::map<std::string, std::string>::value_type(fieldName[index],
                                                                                                       ORM_MYSQL_OP::FieldManager::realTypeString(
                                                                                                               val)));\
                                      index++;
                                      return true;
                                  });
            _nameToType = nameToType;

            _fieldName=fieldName;
            _query = new Query(tableName, *_connector,_fieldName,_nameToType);
            return std::move(*_query);
        }

        template<typename T>
        Select select(T &classObject, Exp expr) {
            std::vector<std::string> fieldName = ORM_MYSQL_OP::FieldManager::extractField<T>();
            std::map<std::string, std::string> nameToType;
            int index = 0;
            classObject._Transfer(ORM_MYSQL_OP::FnVisitor(),
                                  [&nameToType, &index, &fieldName](auto &val) {
                                      nameToType.insert(std::map<std::string, std::string>::value_type(fieldName[index],
                                                                                                       ORM_MYSQL_OP::FieldManager::realTypeString(
                                                                                                               val)));\
                                      index++;
                                      return true;
                                  });
            _nameToType = nameToType;

            std::string tableName = classObject._className;
            _select = new Select(tableName, expr, *_connector, _nameToType);
            return std::move(*_select);
        }

        template<typename T>
        Select select(T &classObject) {
            std::vector<std::string> fieldName = ORM_MYSQL_OP::FieldManager::extractField<T>();
            std::map<std::string, std::string> nameToType;
            int index = 0;
            classObject._Transfer(ORM_MYSQL_OP::FnVisitor(),
                                  [&nameToType, &index, &fieldName](auto &val) {
                                      nameToType.insert(std::map<std::string, std::string>::value_type(fieldName[index],
                                                                                                       ORM_MYSQL_OP::FieldManager::realTypeString(
                                                                                                               val)));\
                                      index++;
                                      return true;
                                  });
            _nameToType = nameToType;

            _fieldName=fieldName;
            std::string tableName = classObject._className;
            _select = new Select(tableName, *_connector, _fieldName, _nameToType);
            return std::move(*_select);
        }

        // a dangerous interface for someone who want to use sql sentances
//        ORM_MYSQL_OP::SQLConnector Executor() {
//            return std::move(*_connector);
//        }



        /************************************************************************/

    private:
        ORM_MYSQL_OP::SQLConnector *_connector;
        std::vector<std::string> _fieldName;
        std::map<std::string, std::string> _nameToType;

        Query *_query;
        Select *_select;

    };

}
#endif //ORM_MYSQL_H
