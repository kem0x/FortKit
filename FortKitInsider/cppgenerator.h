#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <functional>
#include <queue>

namespace CPPGenerator
{
    // https://ctrpeach.io/posts/cpp20-string-literal-template-parameters/
    template <size_t N>
    struct StringLiteral
    {
        constexpr StringLiteral(const char (&str)[N])
        {
            std::copy_n(str, N, value);
        }

        char value[N];
    };

    class Variable
    {
    public:
        std::string type;
        std::string name;
        bool isPtr = false;
        bool isRef = false;
        bool isStatic = false;
        bool isConst = false;
        bool isConstexpr = false;
        bool isStruct = false;
        bool isArray = false;
        int arraySize = 1;
        bool isBitField = false;
        int bitFieldSize = 1;
        std::string comment;

    private:
        Variable(std ::string Type, std ::string Name)
            : type(Type)
            , name(Name)
        {
        }

    public:
        template <StringLiteral lit>
        constexpr static auto New(std::string name) -> Variable
        {
            constexpr auto typeName = lit.value;
            return Variable(typeName, name);
        }

        template <typename T>
        constexpr static auto New(std::string name) -> Variable
        {
            return Variable(typeid(T).name(), name);
        }

        static auto New(std::string type, std::string name) -> Variable
        {
            return Variable(type, name);
        }
    };

    class Header
    {
        bool isInNamespace;
        bool isInStruct;
        bool isInClass;

        std::vector<std::string> generatedStructs;
        std::vector<std::string> generatedClasses;

        std::queue<std::function<void()>> queuedStructs;

        std::string fileName;
        std::stringstream file;

    public:
        Header(std::string _fileName)
            : fileName(_fileName)
        {
        }

        ~Header()
        {
            std::fstream out;
            out.open(fileName + ".h", std::fstream::out | std::fstream::trunc);

            out << file.rdbuf();
            out.close();
        }

        auto AddText(std::string text)
        {
            file << text;
        }

        auto isAlreadyGeneratedStruct(std::string name)
        {
            for (auto&& str : generatedStructs)
            {
                if (str == name)
                {
                    return true;
                }
            }

            return false;
        }

        auto isAlreadyGeneratedClass(std::string name)
        {
            for (auto&& str : generatedClasses)
            {
                if (str == name)
                {
                    return true;
                }
            }

            return false;
        }

        auto queueStruct(std::function<void()> lambda)
        {
            queuedStructs.push(lambda);
        }

        void flushStructs()
        {
            while (!queuedStructs.empty())
            {
                queuedStructs.front()();
                queuedStructs.pop();
            }
        }

        void tab(size_t count)
        {
            for (size_t i = 0; i < count; i++)
            {
                file << "\t";
            }
        }

        void forwardDeclare(std::string name, std::string type = "struct")
        {
            file << type << " " << name << ";\n";
        }

        void pragma(std::string expr)
        {
            file << "#pragma " << expr << "\n"
                 << std::endl;
        }

        void include(std::string fileName, bool useQuotes = true)
        {
            if (useQuotes)
                file << "#include \"" << fileName << "\"" << std::endl;
            else
                file << "#include <" << fileName << ">" << std::endl;
        }

        void namespaceStart(std::string name)
        {
            file << "namespace " << name << "\n{" << std::endl;
            isInNamespace = true;
        }

        void namespaceEnd()
        {
            if (isInNamespace)
            {
                file << "}\n"
                     << std::endl;
                isInNamespace = false;
            }
        }

        void classStart(std::string name, std::string parent)
        {
            generatedClasses.push_back(name);

            if (isInNamespace)
                tab(1);

            file << "class " << name;

            if (!parent.empty())
                file << " : public " << parent;

            file << "\n";

            if (isInNamespace)
                tab(1);

            file << "{" << std::endl;
            isInClass = true;
        }

        void definePublic()
        {
            if (isInClass)
            {
                file << "public:" << std::endl;
            }
        }

        void definePrivate()
        {
            if (isInClass)
            {
                file << "private:" << std::endl;
            }
        }

        void classEnd()
        {
            if (isInClass)
            {
                if (isInNamespace)
                    tab(1);

                file << "};\n"
                     << std::endl;
                isInClass = false;
            }
        }

        void structStart(std::string name, std::string parent = "")
        {
            generatedStructs.push_back(name);

            if (isInClass)
                tab(1);

            if (isInNamespace)
                tab(1);

            file << "struct " << name;

            if (!parent.empty())
                file << " : " << parent;

            file << "\n";

            if (isInClass)
                tab(1);

            if (isInNamespace)
                tab(1);

            file << "{" << std::endl;
            isInStruct = true;
        }

        void structEnd(bool flushQueue = true)
        {
            if (isInStruct)
            {
                if (isInClass)
                    tab(1);

                if (isInNamespace)
                    tab(1);

                file << "};\n"
                     << std::endl;
                isInStruct = false;
            }

            if (flushQueue)
                flushStructs();
        }

        void variable(Variable& var)
        {
            if (isInStruct)
                tab(1);

            if (isInClass)
                tab(1);

            if (isInNamespace)
                tab(1);

            if (var.isConstexpr)
                file << "constexpr ";

            if (var.isConst)
                file << "const ";

            if (var.isStatic)
                file << "static ";

            if (var.isStruct)
                file << "struct ";

            file << var.type;

            if (var.isPtr)
                file << "*";

            if (var.isRef)
                file << "&";

            file << " " << var.name
                 << (var.isArray ? std::format("[0x{:x}]", var.arraySize) : "")
                 << (var.isBitField ? std::format(": {}", var.bitFieldSize) : "")
                 << "; "
                 << (var.comment.empty() ? "" : "// " + var.comment)
                 << std::endl;
        }
    };
}
