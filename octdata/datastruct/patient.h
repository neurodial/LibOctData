#pragma once

#include "substructure_template.h"
#include "study.h"

namespace OctData
{
	class Patient : public SubstructureTemplate<Study>
	{
	public:
		enum class Sex { Unknown, Female, Male};

		std::string getForename() const                          { return forename; }
		std::string getSurname () const                          { return surname ; }
		std::string getTitle   () const                          { return title   ; }
		std::string getId      () const                          { return id      ; }
		Sex         getSex     () const                          { return sex     ; }


		void setForename(const std::string& v)                   { forename = v; }
		void setSurname (const std::string& v)                   { surname  = v; }
		void setTitle   (const std::string& v)                   { title    = v; }
		void setId      (const std::string& v)                   { id       = v; }
		void setSex     (const Sex          v)                   { sex      = v; }

	private:
		std::string forename;
		std::string surname ;
		std::string title   ;
		std::string id      ;

		Sex sex = Sex::Unknown;
	};

}