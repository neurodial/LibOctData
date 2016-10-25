#pragma once

#include "substructure_template.h"
#include "study.h"
#include "date.h"

namespace OctData
{
	class Patient : public SubstructureTemplate<Study>
	{
	public:
		explicit Patient(int internalId) : internalId(internalId) {}

		enum class Sex { Unknown, Female, Male};

		const std::string& getForename () const                  { return forename ; }
		const std::string& getSurname  () const                  { return surname  ; }
		const std::string& getTitle    () const                  { return title    ; }
		const std::string& getId       () const                  { return id       ; }
		Sex                getSex      () const                  { return sex      ; }
		const Date&        getBirthdate() const                  { return birthdate; }


		void setForename (const std::string& v)                  { forename  = v ; }
		void setSurname  (const std::string& v)                  { surname   = v ; }
		void setTitle    (const std::string& v)                  { title     = v ; }
		void setId       (const std::string& v)                  { id        = v ; }
		void setSex      (const Sex          v)                  { sex       = v ; }
		void setBirthdate(const Date&       bd)                  { birthdate = bd; }

		const std::string& getPatientUID() const                 { return uid; }
		void setPatientUID(const std::string& id)                { uid = id  ; }

		      Study& getStudy(int seriesId)                      { return getAndInsert(seriesId)         ; }
		const Study& getStudy(int seriesId) const                { return *(substructureMap.at(seriesId)); }

		int getInternalId() const                                      { return internalId; }
	private:
		const int internalId;

		std::string forename;
		std::string surname ;
		std::string title   ;
		std::string id      ;
		std::string uid     ;

		Date birthdate      ;

		Sex sex = Sex::Unknown;
	};

}
