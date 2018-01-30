#pragma once

#include<ostream>

#include "substructure_template.h"
#include "study.h"
#include "date.h"

#include"objectwrapper.h"

namespace OctData
{
	class Patient : public SubstructureTemplate<Study>
	{
	public:
		explicit Patient(int internalId) : internalId(internalId) {}

		enum class Sex { Unknown, Female, Male};
		typedef ObjectWrapper<Sex> SexEnumWrapper;

		static const char* getSexName(Sex sex);
		const char* getSexName()          const                  { return getSexName(sex); }

		const std::string& getForename () const                  { return forename ; }
		const std::string& getSurname  () const                  { return surname  ; }
		const std::string& getTitle    () const                  { return title    ; }
		const std::string& getId       () const                  { return id       ; }
		Sex                getSex      () const                  { return sex      ; }
		const Date&        getBirthdate() const                  { return birthdate; }

		const std::u16string& getDiagnose() const                { return diagnose;  }


		void setForename (const std::string& v)                  { forename  = v ; }
		void setSurname  (const std::string& v)                  { surname   = v ; }
		void setTitle    (const std::string& v)                  { title     = v ; }
		void setId       (const std::string& v)                  { id        = v ; }
		void setSex      (const Sex          v)                  { sex       = v ; }
		void setBirthdate(const Date&       bd)                  { birthdate = bd; }

		void setDiagnose (const std::u16string& v)               { diagnose  = v ; }

		const std::string& getPatientUID() const                 { return uid; }
		void setPatientUID(const std::string& id)                { uid = id  ; }
		const std::string& getAncestry() const                   { return ancestry; }
		void setAncestry (const std::string& v)                  { ancestry  = v ; }


		      Study& getInsertId(int id)                               { return getAndInsert(id) ; }

		      Study& getStudy(int seriesId)                      { return getAndInsert(seriesId)         ; }
		const Study& getStudy(int seriesId) const                { return *(substructureMap.at(seriesId)); }

		int getInternalId() const                                      { return internalId; }


		template<typename T> void getSetParameter(T& getSet)           { getSetParameter(getSet, *this); }
		template<typename T> void getSetParameter(T& getSet)     const { getSetParameter(getSet, *this); }


		struct PrintOptions
		{
			std::ostream& stream;

			PrintOptions(std::ostream& stream) : stream(stream) {}

			template<typename T>
			void operator()(const char* name, const T& value)
			{
				stream << std::setw(26) << name << " : " << value << '\n';
			}
		};

		void print(std::ostream& stream) const
		{
			PrintOptions printer(stream);
			this->getSetParameter(printer);
		}

	private:
		const int internalId;

		std::string forename;
		std::string surname ;
		std::string title   ;
		std::string id      ;
		std::string uid     ;
		std::string ancestry;

		std::u16string diagnose;

		Date birthdate      ;

		Sex sex = Sex::Unknown;

		template<typename T, typename ParameterSet>
		static void getSetParameter(T& getSet, ParameterSet& p)
		{
			SexEnumWrapper sexWrapper(p.sex);
			DateWrapper    birthDateWrapper(p.birthdate);

// 			getSet("internalId", p.internalId                               );
			getSet("forename"  , p.forename                                 );
			getSet("surname"   , p.surname                                  );
			getSet("title"     , p.title                                    );
			getSet("id"        , p.id                                       );
			getSet("uid"       , p.uid                                      );
			getSet("ancestry"  , p.ancestry                                 );
			getSet("birthdate" , static_cast<std::string&>(birthDateWrapper));
			getSet("sex"       , static_cast<std::string&>(sexWrapper)      );
		}

	};



}
