#pragma once
#include "../include/mozart/any.hpp"
#include <stdexcept>
#include <string>
#include <deque>
#include <map>
namespace cov_basic
{
	enum class token_types
	{
		null,action,signal,id,value
	};
	enum class action_types
	{
		endblock_,endline_,define_,as_,if_,then_,else_,while_,do_,for_,break_,continue_,function_,return_
	};
	enum class signal_types
	{
		add_,sub_,mul_,div_,mod_,pow_,com_,dot_,und_,abo_,asi_,equ_,ueq_,aeq_,neq_,and_,or_,not_,slb_,srb_,mlb_,mrb_,llb_,lrb_
	};
	class token_base
	{
		public:
		token_base()=default;
		token_base(const token_base&)=default;
		virtual ~token_base()=default;
		virtual token_types get_type() const noexcept=0;
	};
	class token_action final:public token_base
	{
		action_types mType;
		public:
		token_action()=delete;
		token_action(action_types t):mType(t){}
		virtual token_types get_type() const noexcept{return token_types::action;}
		action_types get_action() const noexcept{return this->mType;}
	};
	class token_signal final:public token_base
	{
		signal_types mType;
		public:
		token_signal()=delete;
		token_signal(signal_types t):mType(t){}
		virtual token_types get_type() const noexcept{return token_types::signal;}
		signal_types get_signal() const noexcept{return this->mType;}
	};
	class token_id final:public token_base
	{
		std::string mId;
		public:
		token_id()=delete;
		token_id(const std::string& id):mId(id){}
		virtual token_types get_type() const noexcept{return token_types::id;}
		const std::string& get_id() const noexcept{return this->mId;}
	};
	class token_value final:public token_base
	{
		cov::any mVal;
		public:
		token_value()=delete;
		token_value(const cov::any& val):mVal(val){}
		virtual token_types get_type() const noexcept{return token_types::value;}
		const cov::any& get_value() const noexcept{return this->mVal;}
	};
	template<typename Key,typename T>
	class mapping final
	{
		std::map<Key,T> mDat;
		public:
		mapping(std::initializer_list<std::pair<const Key, T>> l):mDat(l){}
		bool exsist(const Key& k)
		{
			return mDat.find(k)!=mDat.end();
		}
		T match(const Key& k)
		{
			if(!exsist(k))
				throw std::logic_error("Undefined Mapping.");
			return mDat.at(k);
		}
	};
	mapping<std::string,signal_types> signal_map={
		{"+",signal_types::add_},{"=",signal_types::asi_},{"&&",signal_types::and_},{"==",signal_types::equ_}
	};
	mapping<std::string,action_types> action_map={
		{"End",action_types::endblock_},{"Define",action_types::define_},{"As",action_types::as_}
	};
	mapping<std::string,bool> boolean_map={
		{"True",true}
	};
	char signals[]={
		'+','=','&'
	};
	bool issignal(char ch)
	{
		for(auto&c:signals)
			if(c==ch)
				return true;
		return false;
	}
	void lexer(const std::deque<char>& buff,std::deque<token_base*>& tokens)
	{
		std::string tmp;
		token_types type=token_types::null;
		bool inside_str=false;
		for(std::size_t i=0;i<buff.size();)
		{
			if(inside_str)
			{
				if(buff[i]=='\"')
				{
					tokens.push_back(new token_value(tmp));
					tmp.clear();
					inside_str=false;
				}else{
					tmp+=buff[i];
				}
				++i;
				continue;
			}
			switch(type)
			{
			case token_types::null:
				if(buff[i]=='\n')
				{
					tokens.push_back(new token_action(action_types::endline_));
					++i;
					continue;
				}
				if(std::isspace(buff[i]))
				{
					++i;
					continue;
				}
				if(signal_map.exsist(std::string(1,buff[i])))
				{
					type=token_types::signal;
					continue;
				}
				if(std::isdigit(buff[i]))
				{
					type=token_types::value;
					continue;
				}
				if(std::isalpha(buff[i]))
				{
					type=token_types::id;
					continue;
				}
				break;
			case token_types::id:
				if(std::isalnum(buff[i])||buff[i]=='_')
				{
					tmp+=buff[i];
					++i;
					continue;
				}
				type=token_types::null;
				if(action_map.exsist(tmp))
				{
					tokens.push_back(new token_action(action_map.match(tmp)));
					tmp.clear();
					break;
				}
				if(boolean_map.exsist(tmp))
				{
					tokens.push_back(new token_value(boolean_map.match(tmp)));
					tmp.clear();
					break;
				}
				tokens.push_back(new token_id(tmp));
				tmp.clear();
				break;
			case token_types::signal:
				if(issignal(buff[i]))
				{
					tmp+=buff[i];
					++i;
					continue;
				}
				type=token_types::null;
				tokens.push_back(new token_signal(signal_map.match(tmp)));
				tmp.clear();
				break;
			case token_types::value:
				if(std::isdigit(buff[i])||buff[i]=='.')
				{
					tmp+=buff[i];
					++i;
					continue;
				}
				if(std::isalnum(buff[i])||buff[i]=='_')
				{
					type=token_types::id;
					tmp+=buff[i];
					++i;
					continue;
				}
				type=token_types::null;
				tokens.push_back(new token_value(std::stold(tmp)));
				tmp.clear();
				break;
			}
			if(buff[i]=='\"')
			{
				inside_str=true;
				++i;
				continue;
			}
		}
		switch(type)
		{
		case token_types::id:
			if(action_map.exsist(tmp))
			{
				tokens.push_back(new token_action(action_map.match(tmp)));
				break;
			}
			if(boolean_map.exsist(tmp))
			{
				tokens.push_back(new token_value(boolean_map.match(tmp)));
				break;
			}
			tokens.push_back(new token_id(tmp));
			break;
		case token_types::signal:
			tokens.push_back(new token_signal(signal_map.match(tmp)));
			break;
		case token_types::value:
			tokens.push_back(new token_value(std::stold(tmp)));
			break;
		}
	}
}