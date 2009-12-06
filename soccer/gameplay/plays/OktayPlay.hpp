/**
 *  Example play: This is a template for a play.
 *  To use, implement the functions and add the necessary member variables
 *  and do a test replacement for ExamplePlay with whatever name you want.
 */

#pragma once

#include "../Play.hpp"

namespace Gameplay
{
	namespace Plays
	{
		class OktayPlay: public Play
		{
			public:
				OktayPlay(GameplayModule *gameplay);

				/** returns true if the play is currently applicable given gamestate */
				virtual bool applicable();

				/** Assigns robots to the play given a set of robots */
				virtual void assign(std::set<Robot *> &available);

				/** Called every frame */
				virtual bool run();

			protected:
				// Insert sub behaviors here as member variables
		};
	}
}
