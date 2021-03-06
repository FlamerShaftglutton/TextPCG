//compile with: g++ -o script_test.exe script_test.cxx src/Scripting.cpp src/Expression.cpp src/string_utils.cpp src/mymath.cpp src/Log.cpp -std=c++14 -g -DDEBUG
//labeled as .cxx to exlude being pulled into the build process when using make

#include "src/Scripting.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct Object
{
	bool visible;
	bool visible_in_short_description;
	bool friendly;
	bool mobile;
	bool playable;
	
	int hitpoints;
	int attack;
	float hit_chance;
	
	std::string description;
	std::string name;
	
	Object()
	{
		visible = false;
		visible_in_short_description = false;
		friendly = false;
		mobile = false;
		playable = false;
		
		hitpoints = 0;
		attack = 0;
		hit_chance = 0.0f;
		
		description = "";
		name = "";
	}
	
	void fill_ObjectMap(ObjectMap& o)
	{
		o.visible = &visible;
		o.visible_in_short_description = &visible_in_short_description;
		o.friendly = &friendly;
		o.mobile = &mobile;
		o.playable = &playable;
		
		o.hitpoints = &hitpoints;
		o.attack = &attack;
		o.hit_chance = &hit_chance;
		
		o.description = &description;
		o.name = &name;
	}
};

void print_sv(ScriptingVariables& sv)
{
	std::cout	<< "\n\nMain Text: " << *(sv.main_text)
				<< "\n\nCurrent_Room: "
				<< "\n\tDescription: " << *(sv.current_room.description)
				<< "\n\tShort Description: " << *(sv.current_room.short_description)
				<< "\n\tMinimap Symbol: " << *(sv.current_room.minimap_symbol);
	for (auto& om : sv.current_room.objects)
	{
		std::cout 	<< "\n\nObject: "
					<< "\n\tVisible: " << *(om.visible)
					<< "\n\tVisible in short description: " << *(om.visible_in_short_description)
					<< "\n\tFriendly: " << *(om.friendly)
					<< "\n\tMobile: " << *(om.mobile)
					<< "\n\tPlayable: " << *(om.playable)
					<< "\n\tHitpoints: " << *(om.hitpoints)
					<< "\n\tAttack: " << *(om.attack)
					<< "\n\tHit Chance: " << *(om.hit_chance)
					<< "\n\tDescription: " << *(om.description)
					<< "\n\tName: " << *(om.name);
	}
}

int main()
{
	std::ifstream infile("expressions.txt");
	
	ScriptingVariables sv;
	std::string main_text = "Some Dummy Text. Overwrite this or append, whatever.\n\n";
	sv.main_text = &main_text;
	std::string description = "A dank, smelly arena. Will this place test you, or will you test it?";
	sv.current_room.description = &description;
	std::string short_description = "An arena";
	sv.current_room.short_description = &short_description;
	std::string minimap_symbol = "<fg=yellow><bg=yellow> ";
	sv.current_room.minimap_symbol = &minimap_symbol;
	
	Object o;
	ObjectMap om;
	o.playable = false;
	o.hitpoints = 11;
	o.description = "A somewhat short man in a dark gray cloak. He mutters to himself while eyeing you.";
	o.name = "Mysterious Stranger";
	o.fill_ObjectMap(om);
	sv.current_room.objects.push_back(om);
	sv.caller = om;
	
	Object o2;
	o2.visible = true;
	o2.visible_in_short_description = true;
	o2.friendly = true;
	o2.mobile = true;
	o2.playable = true;
	o2.hitpoints = 100;
	o2.attack = 10;
	o2.hit_chance = 0.25f;
	o2.description = "An ugly humanoid. It's hideous!";
	o2.name = "You";
	
	o2.fill_ObjectMap(om);
	
	sv.current_room.objects.push_back(om);
	sv.player = om;
	
	std::string all_text = "";
	while (!infile.eof())
	{
		std::string line;
		std::getline(infile,line);
		all_text += line + "\n";
	}
	
	ScriptSet ss;
	ss.construct("",all_text,"");
	
	//print_sv(sv);

	ss.execute_on_sight(sv);

	std::cout << main_text << std::endl;
	
	//print_sv(sv);
	
	return 0;
}