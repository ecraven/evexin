/*
 * Copyright (c) 2013 Kevin Smith
 * Licensed under the GNU General Public License v3.
 * See Documentation/Licenses/GPLv3.txt for more information.
 */

#pragma once

#include <string>
#include <map>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <Eve-Xin/Controllers/Skill.h>
#include <Eve-Xin/Controllers/SkillItem.h>
#include <Eve-Xin/Controllers/SkillLevel.h>

namespace EveXin {
	class SkillPlan : public SkillItem {
		public:
			typedef boost::shared_ptr<SkillPlan> ref;

			SkillPlan(SkillItem::ref parent, const std::string& id, const std::string& name);
			virtual ~SkillPlan();

			/**
			 * Try to add another level of this skill to the plan.
			 * Will insert dependencies.
			 * @return  Skill added.
			 */
			bool addSkill(Skill::ref skill);

			/**
			 * Try to add specified level of this skill to the plan.
			 * Will insert dependencies.
			 * Safe to call with skills that are already trained, just returns false.
			 * @return  Skill added.
			 */
			bool addSkill(Skill::ref skill, int level);

			/**
			 * Set the known skills for this character.
			 * Will remove any skills from the plan that are now known.
			 * Iterates down the tree from the provided root.
			 */
			void setKnownSkills(SkillItem::ref knownSkillRoot);

			virtual std::vector<SkillItem::ref> getChildren() const;
		private:
			std::map<std::string, SkillLevel::ref> knownSkills_; // Flat mapping of known skill levels
			std::map<std::string, SkillLevel::ref> plannedSkills_; // Flat mapping of known skill levels
			std::vector<SkillItem::ref> plan_;
	};
}
