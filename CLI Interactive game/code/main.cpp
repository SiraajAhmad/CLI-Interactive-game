#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "json.hpp"
using namespace std;
using json = nlohmann::json;


void displayGameCommands() {
	cout << "The following list are eligible commands in this game (lowercase) : " << endl << endl;
	cout << "Navigation commands: " << endl;
	cout << "go north / up" << endl;
	cout << "go east / right" << endl;
	cout << "go south / down" << endl;
	cout << "go forward / back / out" << endl << endl;

	cout << "General commands: " << endl;
	cout << "look" << endl;
	cout << "take(item)" << endl;
	cout << "eat" << endl;
	cout << "kill (enemy)" << endl;
	cout << "health" << endl;
	cout << "heal" << endl;
	cout << "inventory (view objects you have picked up)" << endl;
	cout << "exits (prints a list of possible exit commands for each room)" << endl;
	cout << "quit (quit the game)" << endl;
};

int main(int argc, char* argv[]) {
	//if command line arguments are not the two expected (main and filename.json), display exception message.
	if (argc != 2) {
		cerr << "Usage: " << argv[0] << " <filename.json>" << endl;
		return 1;
	}

	ifstream file(argv[1]);
	if (!file.is_open()) {
		cerr << "Error could not open file" << endl;
	}

	json jsonData;
	try {
		//load all json file data in object jsonData. 
		file >> jsonData;
	}
	catch (const exception e) {
		cerr << "Error while parsing JSON" << e.what() << endl;
	}
	//All initialization variables
	//Create variables relating to user. 
	vector<string> playerInventory;//Create vector to store objects the player picks up
	int playerHealth = 100; //variable to decide players health based off enemy attacks
	bool isPlayerAlive = true; //bool to control game, game should end if bool isplayerAlive = false.

	vector<string> deadEnemies;//Create vector to store dead enemies
	string initialRoomId = jsonData["player"]["initialroom"].get<string>();//retrieve initial room id
	string currentRoomDescription; 
	string currentRoomId = initialRoomId;//set initial current room to the initial room when map loads

	//retrieves information about game objectives
	string gameObjectiveType = jsonData["objective"]["type"].get<string>();//retrieves the type of game objective(kill, collect or reach room)
	string gameObjectiveDestination;
	vector<string> collectedObjectiveObjects;//Create vector to store collected Objects

	//Welcome user to game, display objectives and display information about initial room
	cout << endl << "Welcome to my text-adventure game!" << endl << endl;
	cout << "The aim of the game is to: " << endl;
	//filter output depending on the type of objective. Loop through array to print objectives
	if (gameObjectiveType == "room") {
		for (const auto arrayElement : jsonData["objective"]["what"]) {
			cout << "-Reach " << arrayElement.get<string>() << endl;
			gameObjectiveDestination = arrayElement.get<string>();
		}
	}
	//if type is collect or kill, display the objects to collect/enemies to collect. 
	else {
		for (const auto arrayElement : jsonData["objective"]["what"]) {
			cout << "-" << gameObjectiveType << " the " << arrayElement.get<string>() << endl;
		}
	}

	//retrieve current room and output description
	cout << endl << "You are currently in " << currentRoomId << "." << endl << endl;
	string initialRoomDesc = jsonData["rooms"][0]["desc"];
	cout << initialRoomDesc << endl << endl;

	//Output if there are any objects in the room
	for (auto object : jsonData["objects"]) {
		if (object["initialroom"] == currentRoomId) {
			cout << "Objects in the room: " << endl;
			cout << object["id"].get<string>() << "- " << object["desc"].get<string>() << endl << endl;
		}
	}

	cout << endl << "Enter 'commands' to see a list of eligible text commands. " << endl;
	cout << "Enter 'quit' to quit the game at any time." << endl << endl;

	//create bool to use for looping until the game has been won
	bool hasObjectiveBeenCompleted = false;

	while (hasObjectiveBeenCompleted == false && isPlayerAlive == true) {
		//code to split the input into the command and the object the user wants to interact with.
		//the object is stored in var objectName
		string userInput;
		cout << ">";
		getline(cin, userInput);
		istringstream iss(userInput);
		string command, targetName;//target name is generalised, can refer to enemies,objects or directions
		iss >> command;

		//display commands upon request
		if (command == "commands") {
			displayGameCommands();
		}
		//quits game upon command
		else if (command == "quit") {
			cout << "You have chosen to quit the game!" << endl;
			exit(1);
		}
		//displays information about the current room the player is in
		else if (command == "look") {
			for (auto rooms : jsonData["rooms"]) {
				if (rooms["id"] == currentRoomId) {
					currentRoomDescription = rooms["desc"];
					cout << currentRoomDescription << endl << endl;
				}
			}
			//Output if there are any objects in the room
			//verifies that the current room is equal to the initial object room AND the object is not in the users INVENTORY
			for (auto object : jsonData["objects"]) {
				if (object["initialroom"] == currentRoomId and find(playerInventory.begin(), playerInventory.end(), object["id"]) == playerInventory.end()) {
					cout << "Objects in the room: " << endl;
					cout << object["id"].get<string>() << "- " << object["desc"].get<string>() << endl << endl;
				}
			}
			//output if there are any enemies in the room
			for (auto enemies : jsonData["enemies"]) {
				//check if enemy exists in room and is not dead already
				if (enemies["initialroom"] == currentRoomId && find(deadEnemies.begin(), deadEnemies.end(), enemies["id"]) == deadEnemies.end()) {
					//prints enemy name and a description
					cout << "Enemies in the room: " << endl;
					cout << enemies["id"].get<string>() << "- " << enemies["desc"].get<string>() << endl << endl;

					//Prints aggressiveness of enemy
					int agg = enemies["aggressiveness"].get<int>();
					cout << "The " << enemies["id"].get<string>() << " has an aggressiveness of " << agg << " out of 100." << endl;
					
					//store all the objects an enemy can be killed by in a vector and loop through and output them
					cout << "It can be killed by: " << endl;
					vector<string> v = enemies["killedby"].get<vector<string>>();
					for (string s : v) cout << s << endl;
				}
			}
		}
		//executes the take command, removes the object from the initial room and inserts it into a vector (player inventory)
		else if (command == "take") {
			getline(iss >> ws, targetName);
			bool objectFound = false;
			//this stores any objects which need to be collected to complete the game. 
			auto objectiveObjects = jsonData["objective"]["what"];

			//check if the item instructed to be taken exists in the current room and if it is not already in the player inventory, allow the user to pick it up
			for (auto object : jsonData["objects"]) {
				if (currentRoomId == object["initialroom"] && object["id"] == targetName && find(playerInventory.begin(), playerInventory.end(), object["id"]) == playerInventory.end()) {
					objectFound = true;
					cout << "You have picked up " << targetName << ". It can be found in your inventory. Use command 'inventory' to see your objects" << endl << endl;
					//add it to player inventory and and remove from current room
					object["initialroom"] = nullptr;
					playerInventory.push_back(targetName);
					collectedObjectiveObjects.push_back(targetName);
				}
				//check if all objects which need to be collected have been taken, if true, end game
				if (gameObjectiveType == "collect") {
					bool objectiveCompleted = true;
					for (auto object : objectiveObjects) {
						bool ifFound = false;
						for (auto collectedObject : collectedObjectiveObjects) {
							if (object == collectedObject) {
								ifFound = true;
								break;
							}
						}
						if (!ifFound) {
							objectiveCompleted = false;
						}
					//end game if objective completed boolean is set to true
					}
					if (objectiveCompleted) {
						cout << "Congratulations! You have collected all objects and completed your mission. You have won the Game!!!" << endl;
						hasObjectiveBeenCompleted = true;
						break;
					}
				}
			}
			if (!objectFound) {
				cout << "This object does not exist in this room. Try another command" << endl;
			}
		}

		//output the available exits in a room and their destination if the user requests "exits"
		else if (command == "exits") {

			const auto rooms = jsonData["rooms"];
			for (const auto room : rooms) {
				if (room["id"] == currentRoomId) {
					const auto exits = room["exits"];
					//if there are available exits, loop through and output them to the user
					if (!exits.empty()) {
						cout << "Available Exits from " << currentRoomId << ":" << endl;
						for (auto roomExit = exits.begin(); roomExit != exits.end(); roomExit++) {
							//this gives them the correct exit command to leave as well as the destination it leads to 
							cout << "-" << roomExit.key() << " to " << roomExit.value().get<string>() << "." << endl;
						}
					}
					else {
						cout << "No exit available" << endl;
					}
				}
			}
		}

		else if (command == "go" || command == "move") {
			//retrieve room exit name inputted by user, stored in targetName
			getline(iss >> ws, targetName);

			bool hasEnemyBeenFound = false;
			bool hasRoomChanged = false;

			const auto rooms = jsonData["rooms"];
			for (const auto& room : rooms) {
				if (room["id"] == currentRoomId) {
					const auto& exits = room["exits"];
					//including bool in condition to ensure only one room change is permitted. 
					if (exits.find(targetName) != exits.end() && !hasRoomChanged) {
						string newRoom = exits[targetName];

						// check for enemies to be killed before room changes
						for (auto enemy : jsonData["enemies"]) {
							if (enemy["initialroom"] == currentRoomId && find(deadEnemies.begin(), deadEnemies.end(), enemy["id"]) == deadEnemies.end() && currentRoomId != newRoom) {
								hasEnemyBeenFound = true;
								int aggressiveness = enemy["aggressiveness"];

								// enemy attacks based on aggressiveness
								if (aggressiveness < 19) {
									hasEnemyBeenFound = false;//if an enemy is not aggressive then allow the user to leave the room without being attacked
								}
								else if (aggressiveness >= 20 && aggressiveness <= 45) {
									hasEnemyBeenFound = true;
									playerHealth = playerHealth - 25;//slightly reduce player health 
									cout << "The " << enemy["id"] << " has attacked you and reduced your health by 25 HP!" << endl;
									cout << "Current Health: " << playerHealth << endl << endl;
									cout << "HINT- Try to kill the " << enemy["id"] << " before attempting to leave the room!" << endl << endl;
									if (playerHealth == 0) {
										cout << "The " << enemy["id"] << " attacked and you suffered a slow painful death!" << endl;
										cout << "The game is now OVER! (Next time- try to kill the " << enemy["id"] << " before leaving the room!" << endl << endl;
										isPlayerAlive = false;
									}
								}
								else if (aggressiveness >= 46 && aggressiveness <= 80) {
									hasEnemyBeenFound = true;
									playerHealth = playerHealth - 50;//damage player health by half
									cout << "The " << enemy["id"] << " has attacked you and reduced your health by 50 HP!" << endl;
									cout << "Current Health: " << playerHealth << endl;
									cout << "HINT- Try to kill the " << enemy["id"] << " before attempting to leave the room!" << endl;
									if (playerHealth == 0) {
										cout << "The " << enemy["id"] << " attacked and you were killed!" << endl;
										cout << "The game is now OVER! (Next time- try to kill the " << enemy["id"] << " before leaving the room!" << endl << endl;
										isPlayerAlive = false;
									}
								}
								else if (aggressiveness >= 81 && aggressiveness <= 100) {
									hasEnemyBeenFound = true;
									cout << "The " << enemy["id"] << " attacked and you were instantly killed!" << endl;
									cout << "The game is now OVER! (Next time- try to kill the " << enemy["id"] << " before leaving the room!" << endl << endl;
									playerHealth = 0;//instantly kill player
									isPlayerAlive = false;
								}
								if (!isPlayerAlive) {
									// exit loop if the player is killed, end game
									isPlayerAlive = false;
									break;
								}
							}
							else {
								hasEnemyBeenFound = false;
							}
						}

						// check if room change should occur
						if (!hasEnemyBeenFound) {
							//if the current room = the room states in the the json file as a target destination, display success message and end game
							if (newRoom == gameObjectiveDestination) {
								cout << "Congratulations! You have reached your goal destination!" << endl << endl;
								hasObjectiveBeenCompleted = true;
								break;
							}
							else {
								//update current room 
								cout << "You have entered " << newRoom << "!" << endl << endl;
								cout << "Enter 'look' for a description of this room!" << endl << endl;
								currentRoomId = newRoom;
							}
							hasRoomChanged = true;//update boolean to ensure that a room change can only occur once per command entered
						}
					}
				}
			}

			if (hasEnemyBeenFound) {
				cout << "You cannot leave the room without killing the enemy." << endl << endl;
			}
		}

		//implement killing enemies
		else if (command == "kill") {
			getline(iss >> ws, targetName);
			//create booleans to track progress and to stop conditions based on inputs
			bool hasEnemyBeenKilled = false;
			bool isThereEnemyInRoom = false;
			bool playerHasItemsRequiredToKill = true;

			auto objectiveEnemies = jsonData["objective"]["what"];
			//set enemyinroom to true if there is an enemy in the current room matching name and inital room to current room
			for (auto enemy : jsonData["enemies"]) {
				if (currentRoomId == enemy["initialroom"] && enemy["id"] == targetName) {
					isThereEnemyInRoom = true;
					int enemyAggressiveness = enemy["aggressiveness"].get<int>();
					//loop through object to see whether the player has picked up required objects to kill enemies
					for (auto itemToKill : enemy["killedby"]) {
						bool ItemRequiredToKill = false;
						for (auto inventoryItem : playerInventory) {
							if (inventoryItem == itemToKill) {
								ItemRequiredToKill = true;
								break;
							}
						}
						if (!ItemRequiredToKill) {
							playerHasItemsRequiredToKill = false;
							break;
						}
					}
					//if player has required items, kill enemy and remove the enemy from the room.
					//it will be stored in deadEnemies vector to compare with game objects to see whether the user has completed game requirements. 
					if (playerHasItemsRequiredToKill == true) {
						cout << "You killed the " << targetName << "!" << endl;
						enemy["initialroom"] = nullptr;
						deadEnemies.push_back(targetName);
						if (gameObjectiveType == "kill") {
							bool objectiveCompleted = true;
							for (auto enemy : objectiveEnemies) {
								bool ifFound = false;
								for (auto deadEnemy : deadEnemies) {
									if (enemy == deadEnemy) {
										ifFound = true;
										break;
									}
								}
								if (!ifFound) {
									objectiveCompleted = false;
								}
							}
							//if the user has killed all enemies stated in the json file, complete the game and display success message
							if (objectiveCompleted) {
								cout << "Congratulations! You have killed all the enemies and completed your mission. You have won the Game!!!" << endl;
								hasObjectiveBeenCompleted = true;
							}
						}
					}
					//if user doesnt have required items, kill player and end game. 
					else {
						cout << "You did not have the items required to kill the " << targetName << "!" << endl;
						cout << "You have been attacked and killed! The game is now OVER!!!" << endl;
						isPlayerAlive = false;
					}
					break;
				}
			}
			if (!isThereEnemyInRoom) {
				cout << "There is no enemy in this room. Try another command" << endl;
			}
		}
		//displays the players inventory
		else if (command == "inventory") {
			cout << "Player Inventory: " << endl;
			if (playerInventory.empty()) {
				cout << "Inventory is empty!" << endl;
			}
			else {
				for (const auto object : playerInventory) {
					cout << "-" << object << endl;
				}
			}
		}
		//outputs players health
		else if (command == "health") {
			cout << "Player Health: " << playerHealth << endl;
		}
		//implements healing commands
		else if (command == "heal") {
			//find if the user has bandages in their inventory(only for hauntedhouse.json map)
			auto hasBandages = find(playerInventory.begin(), playerInventory.end(), "bandages");
			if (hasBandages != playerInventory.end()) {
				if (playerHealth != 100) {
					playerHealth = 100;
					cout << "You have healed back to full health!" << endl;
					cout << "Current Health: " << playerHealth << endl;

					//remove bandages from inventory once used. 
					playerInventory.erase(hasBandages);
				}
				else {
					cout << "You are already full health!" << endl;
				}
			}
			else {
				cout << "You do not have any healing items! " << endl;
			}
		}
		//if a user enters a command that is not valid or recognised, prompt them to enter another command
		else {
			cout << "" << endl;
		}
	}
}