#pragma once
#define _WINSOCKAPI_ //does not include winsock in windows.h
//#include <Windows.h>
#include <irrKlang.h>
#include "Assimp.h"
#include "Camera.h"
#include "Ringbuffer.h"
#include <vector>
#include <cstdio>
#include <glfw3.h>
#include "Shader.h"
#include <gtc/type_ptr.hpp>
#include "Arrow.h"
#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <queue>
namespace Game {
#pragma region Util
	#define vectorAdapter(vec) (irrklang::vec3df {(vec).x, (vec).y, (vec).z})
	unsigned int loadTexture(char const *path, bool repeat = false)
	{
		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format; //determines format
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			if (nrComponents == 4 && repeat == false) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			else {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			printf("Texture failed to load at path: %s \n", path);
			stbi_image_free(data);
		}

		return textureID;
	}
	static unsigned int loadSky(std::vector<std::string> faces) {
		unsigned int texture;
		printf_s("allocated texture \n");
		glGenTextures(1, &texture);
		if (texture == NULL) {
			printf_s("Texture is null \n");
			system("pause");
		}
		else {
			printf_s("Texture is not null \n");
		}
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
		int width, height, nrChannels;
		for (unsigned int i = 0; i < faces.size(); i++) {
			unsigned char * data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			if (data) {
				printf_s("data is not null \n");
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				stbi_image_free(data);
			}
			else {
				printf_s("Cubemap failes to load: %s\n", faces[i]);
				stbi_image_free(data);
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		printf("set texture parameters \n");
		return texture;
	}
	class Skybox {
	private:
		float skyboxVertices[144] = {
			// positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f, 1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f,  1.0f, 1.0f,
			1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f,  1.0f
		};
		unsigned int skyVAO, skyVBO;
		unsigned int skyCube;
	public:
		Skybox(std::string baseName) {
			glGenVertexArrays(1, &skyVAO);
			glGenBuffers(1, &skyVBO);
			glBindVertexArray(skyVAO);
			glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
			glEnableVertexAttribArray(0);
			std::vector<std::string> faces;
			faces.push_back("Assets/skybox/" + baseName + "_rt.tga");
			faces.push_back("Assets/skybox/" + baseName + "_lf.tga");
			faces.push_back("Assets/skybox/" + baseName + "_up.tga");
			faces.push_back("Assets/skybox/" + baseName + "_dn.tga");
			faces.push_back("Assets/skybox/" + baseName + "_bk.tga");
			faces.push_back("Assets/skybox/" + baseName + "_ft.tga");
			skyCube = loadSky(faces);
			glBindVertexArray(0);
		}
		void draw() {
			glDepthMask(GL_FALSE);
			glBindVertexArray(skyVAO);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyCube);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
			glDepthMask(GL_TRUE);

		}
	};
	class Quad {
	private:
		unsigned int texture;
		unsigned int normal;
		unsigned int specular;
		float vertices[48] =
		{
			25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
			-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
			-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

			25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
			-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
			25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
		};
		unsigned int VAO, VBO;
	public:
		Quad(char * texture = "none") {
			if (strcmp(texture, "none") != 0) {
				this->texture = loadTexture(texture);
			}
			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glBindVertexArray(0);
		}
		void init(char * texture = "none") {
			if (strcmp(texture, "none") != 0) {
				this->texture = loadTexture(texture);
			}
			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glBindVertexArray(0);
		}
		void draw() {
			glBindVertexArray(VAO);
			if (texture != NULL) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, texture);
				if (normal != NULL) {
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, normal);
				}
				if (specular != NULL) {
					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, specular);
				}
			}
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}
		void addTexture(char * texture, char * type) {
			if (strcmp(type, "normal") != 0) this->normal = loadTexture(texture);
			else if (strcmp(type, "specular") != 0) this->specular = loadTexture(texture);
		}
	};
	struct Player {
	private:
		double time, deathTime;
		bool damage;
		bool black;
		bool clearUp;
		bool shake;
		glm::vec3 respawnLocations[7] = { 
			glm::vec3(0, -2, 0), glm::vec3(41.9, -2, 21.4), glm::vec3(11.4, -2, 35.8),
			glm::vec3(-20.5, -2, 21.7), glm::vec3(-23.4, -2, -31), glm::vec3(14, -2, -21.9),
			glm::vec3(40, -2, -7)
		};
	public:
		char health;
		uint16_t pid;
		unsigned char arrows;
	public:
		Player() : damage(false), time(0), clearUp(false), shake(false), black(false), health(100) {}
		inline void takeDamage() {
			time = glfwGetTime();
			damage = true;
			shake = true;
		}
		inline bool getDamage() {
			if (glfwGetTime() - 3 > time) {
				damage = false;
				clearUp = true;
			}
			if (clearUp && glfwGetTime() - 5 > time) {
				clearUp = false;
			}
			return damage;
		}
		inline bool getShake() {
			if (glfwGetTime() - 0.5 > time) {
				shake = false;
			}
			return shake;
		}
		inline bool isClearing() {
			return clearUp;
		}
		inline float getShakeTime() {
			if (shake) {
				return glfwGetTime() - time;
			}
			else {
				return 0;
			}
		}
		glm::vec3 getRespawnLocation() {
			srand(glfwGetTime());
			int index = rand() % 7;
			deathTime = glfwGetTime();
			black = true;
			return respawnLocations[index];
		}
		inline bool isBlack() {
			if (black) {
				black = glfwGetTime() - deathTime > 2 ? false : true;
			}
			return black;
		}
	};
	class Marker {
	private:
		unsigned int VBO, VAO;
		float * points;
		unsigned int size;
		Shader * markerShader;
	public:
		~Marker() {
			free(points);
			delete markerShader;
		}
		Marker(Shader * markerShader, int amount) {
			this->markerShader = markerShader;
			size = amount;
			points = (float*)calloc(amount * 3, sizeof(float));
			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, size * 3 * sizeof(float), points, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
			glEnableVertexAttribArray(0);
		}
		void draw(glm::mat4 model, glm::mat4 projection, glm::mat4 view) {
			USE_SHADER(markerShader->id);
			SHADER_SET_MAT4(markerShader->id, "model", model);
			SHADER_SET_MAT4(markerShader->id, "projection", projection);
			SHADER_SET_MAT4(markerShader->id, "view", view);
//			markerShader->setVec3("color", glm::vec3(0, 191, 255));
			glBindVertexArray(VAO);
			glPointSize(20.f);
			glDrawArrays(GL_POINTS, 0, size);
			glBindVertexArray(0);
		}
	};
	class Debug {
	private:
		unsigned int * buffer;
		unsigned int * EBO;
		unsigned int * VAO;
		float ** points;
		unsigned int ** indices;
		unsigned int * size;
		unsigned int * idSize;
		unsigned int totalSize;
		struct Matrices {
			glm::mat4 projection;
			glm::mat4 view;
			glm::mat4 model;
		} matrices;
	public:
		~Debug() {
			delete[] points;
			delete[] indices;
			delete[] buffer;
			delete[] EBO;
			delete[] VAO;
			delete[] size;
			delete[] idSize;
		}
		Debug(const std::vector<std::vector<glm::vec3>> points, const std::vector<std::vector<unsigned int>> & indices) {			
			this->points = new float*[points.size()];
			totalSize = points.size();
			size = new unsigned int[points.size()];
			for (int i = 0; i < points.size(); i++) {
				this->points[i] = new float[points[i].size() * 3];
				size[i] = points[i].size();
				for (int j = 0; j < points[i].size(); j++) {
					this->points[i][j] = points[i][j].x;
					this->points[i][j+1] = points[i][j].y;
					this->points[i][j+2] = points[i][j].z;
				}
				
			}
			this->indices = new unsigned int*[indices.size()];
			idSize = new unsigned int[indices.size()];
			for (int i = 0; i < indices.size(); i++) {
				this->indices[i] = new unsigned int[indices[i].size()];
				idSize[i] = indices[i].size();
				for (int j = 0; j < indices[i].size(); j++) {
					this->indices[i][j] = indices[i][j];
				}
			}
			VAO = new unsigned int[indices.size()];
			buffer = new unsigned int[indices.size()];
			EBO = new unsigned int[indices.size()];
			for (int i = 0; i < indices.size(); i++) {
				glGenVertexArrays(1, &VAO[i]);
				glGenBuffers(1, &buffer[i]);
				glGenBuffers(1, &EBO[i]);

				glBindVertexArray(VAO[i]);

				glBindBuffer(GL_ARRAY_BUFFER, buffer[i]);
				glBufferData(GL_ARRAY_BUFFER, size[i] * sizeof(float), &this->points[i][0], GL_STATIC_DRAW);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, idSize[i] * sizeof(unsigned int), &this->indices[i][0], GL_STATIC_DRAW);

				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
				glEnableVertexAttribArray(0);

				glBindVertexArray(0);
			}

		}
		void setMatrices(glm::mat4 projection, glm::mat4 view, glm::mat4 model) {
			matrices.model = model;
			matrices.projection = projection;
			matrices.view = view;
		}
		void draw(Shader debugShader, Shader & normalShader) {
			USE_SHADER(debugShader);
			SHADER_SET_MAT4(debugShader, "projection", matrices.projection);
			SHADER_SET_MAT4(debugShader, "view", matrices.view);
			SHADER_SET_MAT4(debugShader, "model", glm::mat4());
			for (int i = 0; i < totalSize; i++) {
				glBindVertexArray(VAO[i]);
				glPointSize(10.0f);
				//			glDrawArrays(GL_POINTS, 0, size / 3);
				//			glDrawArrays(GL_LINES, 0, (size / 3) / 2);
				//			glDisable(GL_DEPTH_TEST);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				//			glDrawArrays(GL_TRIANGLES, 0, (size / 3) / 3);
				glDrawElements(GL_TRIANGLES, idSize[i], GL_UNSIGNED_INT, 0);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				//			glEnable(GL_DEPTH_TEST);
				glBindVertexArray(0);
			}
			USE_SHADER(normalShader);
		}
	};
#pragma endregion
	std::mutex arrowMutex;
	std::mutex timeMutex;
	std::mutex positionMutex;
	std::mutex backupMutex;
	std::mutex deathMutex;
	class threadLock {
	private:
		std::mutex mu;
		bool isLocked;
	public:
		threadLock(): isLocked(false) {}
		operator bool() {
			std::lock_guard<std::mutex> guard(mu);
			return isLocked;
		}
		void operator=(bool value) {
			std::lock_guard<std::mutex> guard(mu);
			isLocked = value; 
		}
	};
#define UNLOCKED(ThreadLock) (!ThreadLock)
#define LOCKED(ThreadLock) (ThreadLock)
#define THREAD_UPDATE_MULTIPLE 1
#define THREAD_UPDATE_SINGLE 0
#define THREAD_WAIT_UNTIL(ThreadLock) while(ThreadLock) {}
	class threadUpdate {
	private:
		std::mutex mu;
		bool type;
		bool notification;
		int notifications;
	public:
		threadUpdate() : notification(0), type(false) {}
		threadUpdate(bool type) : notification(0), notifications(0), type(type) {}
		void notify() {
			std::lock_guard<std::mutex> guard(mu);
			if (type) notifications++;
			else notification = true;
		}
		void handleNotification() {
			std::lock_guard<std::mutex> guard(mu);
			if (type) notifications--;
			else notification = false;
		}
		int getNotifications() {
			std::lock_guard<std::mutex> guard(mu);
			if (type) return notifications;
			return notification == true ? 1 : 0;
		}
	};
	class Scene {
	private:
		int kills = 0;
		int leaderKills = 0;
		std::string leaderName = "";
		double serverTime = 0;
		double pcTime = 0;
		Arrow deathArrow;
		std::string username = "";
		bool isTimed = true;
		t_clock time;
		std::vector<arrow_packet> serverArrows;
		position user;
		std::vector<position> positions;
	protected:
		threadLock playerLock, arrowLock, serverArrowLock, leaderLock, killsLock, backupLock, nameLock, timeLock, commandLock;
		threadUpdate playerCond, arrowCond, leaderCond, killsCond, deathCond, leaveCond, commandCond, backupCond, newArrow;
		Ringbuffer<Arrow> * arrows;
		irrklang::ISoundEngine * soundEngine;
		char * name;
		std::vector<position> players;
		std::vector<double> playerMoves;
		int err;
		std::vector<Arrow> allArrows;
		std::vector<Arrow> lastServerArrows;
		std::queue<unsigned long long> arrowIds;
		std::queue<arrow_packet> arrowsToBeAdded;
		int lastKills, lastLeaderKills;
		std::string lastLeaderName, lastName;
		t_clock lastTime;
		std::string newCommand = "";
		bool done = false;
		bool msgDone = false;
	public:
		Player player;
		bool connected;
	public:
		void conversationThread() {
			client::getPid(player.pid);
			while (true) {
				printf("Loop \n");
//				clock_t startTime = clock();
				#ifndef UDP
					client::startLoop();
				#endif
				t_clock local_time;
				double local_serverTime;
				position local_user;
				std::vector<position> local_positions;
				std::vector<arrow_packet> local_serverArrows;
				int local_kills, local_leaderKills;
				std::string local_name, local_leaderName;
				arrow_packet local_deathArrow;
				bool local_died = false;
				printf("0 \n");
				if (deathCond.getNotifications()) {
					local_deathArrow = deathArrow.toPacket();
					local_died = true;
					deathCond.handleNotification();
				}
				printf("0.5 \n");
				client::getData(local_kills, local_leaderKills, local_leaderName, local_time, local_serverTime, local_name, local_positions, local_serverArrows);
				printf("0.7 \n");
				if (UNLOCKED(timeLock)) {
					timeLock = true;
					time = local_time;
					printf("Time: %d : %d \n", local_time.minutes, local_time.seconds);
					timeLock = false;
				}

				std::unique_lock<std::mutex> timeLock(timeMutex);
				serverTime = local_serverTime;
				pcTime = glfwGetTime(); //glfwGetTime() is thread safe
				timeLock.unlock();

				std::unique_lock<std::mutex> arrLock(arrowMutex);
				if (arrowIds.size() <= 5) {
					client::getNewArrowId(arrowIds);
				}
				arrLock.unlock();

				printf("1 \n");
				THREAD_WAIT_UNTIL(playerLock);
				playerLock = true;
				positions = local_positions;
				playerCond.notify();
				playerLock = false;
				
				printf("2 \n");
				std::unique_lock<std::mutex> posLock(positionMutex);
				local_user = user;
				posLock.unlock();

				printf("3 \n");
				THREAD_WAIT_UNTIL(arrowLock);
				arrowLock = true;
				std::vector<arrow_packet> sendToArrows;
				for (auto it = arrows->begin(); it != arrows->end(); it++) {
					sendToArrows.push_back((*it).toPacket());
				}
				arrowLock = false;
				
				if (backupCond.getNotifications()) {
					printf("4 \n");
					THREAD_WAIT_UNTIL(backupLock);
					if (arrowsToBeAdded.size() > 0) {
						while (!arrowsToBeAdded.empty()) {
							sendToArrows.push_back(arrowsToBeAdded.front());
							arrowsToBeAdded.pop();
						}
					}
					backupCond.handleNotification();
				}
				printf("5 \n");
				client::sendPlayerData(local_user, sendToArrows, local_died, local_deathArrow);
				printf("6 \n");
				THREAD_WAIT_UNTIL(serverArrowLock);
				serverArrowLock = true;
				serverArrows = local_serverArrows;
				arrowCond.notify();
				serverArrowLock = false;

				if (UNLOCKED(killsLock)) {
					killsLock = true;
					kills = local_kills;
					killsCond.notify();
					killsLock = false;
				}
				
				if (UNLOCKED(leaderLock)) {
					leaderLock = true;
					leaderKills = local_leaderKills;
					leaderName = local_leaderName;
					leaderCond.notify();
					leaderLock = false;
				}
				
				if (UNLOCKED(nameLock)) {
					nameLock = true;
					username = local_name;
					nameLock = false;

				}
				printf("7 \n");
				if (commandCond.getNotifications() && commandLock == false) {
					commandLock = true;
					client::sendCommand(newCommand);
					commandCond.handleNotification();
					commandLock = false;
				}
//				/*
				if (leaveCond.getNotifications()) {
					client::shutdown();
					break;
				}
				printf("8 \n");
//				if(!msgDone) printf("Messaging loop took: %f \n", (clock() - startTime) / (double)CLOCKS_PER_SEC);
//				msgDone = true;
//				*/
			}

		}
	protected:
		void setPos(position & pos) {
			std::lock_guard<std::mutex> posLock(positionMutex);
			user = pos;
		}
		void died(Arrow & died) {
			std::lock_guard<std::mutex> deathLock(deathMutex);
			deathArrow = died;
			deathCond.notify();
		}
		void loadPlayers() {
			if (UNLOCKED(playerLock)) {
				playerLock = true;
				players = positions;
				playerLock = false;
			}
		}
/*		void timing() {
			client::startLoop();
			if (/*UNLOCKED(timeLock) && isTimed) {
//				timeLock = true;
				client::getTime(time);
				if (time.minutes == -2 && time.seconds == 0) isTimed = false;
//				timeLock = false;
			}
//			std::unique_lock<std::mutex> timeLock(timeMutex);
			client::getServerTime(serverTime);
			pcTime = glfwGetTime(); //glfwGetTime() is thread safe
//			timeLock.unlock();
		}*/
	public:
		Scene() {
			soundEngine = irrklang::createIrrKlangDevice();
			soundEngine->play2D("Assets/sounds/blank.wav"); //irrklang lags on first play
		}
		~Scene() {
			delete arrows;
		}
		double getTime() {			
			if (connected) {
				std::lock_guard<std::mutex> guard(timeMutex);
				double offset = serverTime - pcTime;
				return glfwGetTime() + offset;
			}
			return glfwGetTime();
		}
		void getTimer(t_clock & tclock) {
			if (connected) {
				if (UNLOCKED(timeLock)) {
					timeLock = true;
					lastTime = time;
					timeLock = false;
				}
				tclock = lastTime;
			}
			else {
				tclock = { -2, 0 };
			}
		}
		void sendCommand(std::string ncommand) {
			if (connected) {
				if (UNLOCKED(commandLock)) {
					commandLock = true;
					newCommand = ncommand;
					commandCond.notify();
					commandLock = false;
				}
			}
		}
		inline void shutdown() {
			leaveCond.notify();
		}
		virtual void addArrow(Camera & cam) = 0;
		inline bool getDamage() {
			return player.getDamage();
		}
		inline int getHealth() {
			return player.health;
		}
		inline unsigned char & getArrows() {
			return player.arrows;
		}
		inline bool isClearing() {
			return player.isClearing();
		}
		inline bool getShake() {
			return player.getShake();
		}
		inline bool isDead() {
			return player.isBlack();
		}
		inline float getShakeTime() {
			return player.getShakeTime();
		}
		inline void setHealth(int health) {
			player.health = health;
		}
		
		int getKills() {
			if (!connected) return 0;
			if (UNLOCKED(killsLock) && killsCond.getNotifications()) {
				killsLock = true;
				lastKills = kills;
				killsCond.handleNotification();
				killsLock = false;
			}
			return lastKills;
		}
		void getLeader(int & leaderKills, std::string & leaderName) {
			if (!connected) {
				leaderKills = 0;
				leaderName = "offline";
				return;
			}
			if (UNLOCKED(leaderLock) && leaderCond.getNotifications()) {
				leaderLock = true;
				lastLeaderKills = this->leaderKills;
				lastLeaderName = this->leaderName;
				leaderCond.handleNotification();
				leaderLock = false;
			}
			leaderKills = lastLeaderKills;
			leaderName = lastLeaderName;
		}
		std::vector<Arrow> getServerArrows() {
			if (UNLOCKED(serverArrowLock)) {
				serverArrowLock = true;
				lastServerArrows.erase(lastServerArrows.begin(), lastServerArrows.end());
				for (int i = 0; i < serverArrows.size(); i++) {
					lastServerArrows.push_back(serverArrows[i]);
				}
				serverArrowLock = false;
			}
			return lastServerArrows;
		}
		std::string getName() {
			if (!connected) return "";
			if (UNLOCKED(nameLock)) {
				nameLock = true;
				lastName = username;
				nameLock = false;
			}
			return lastName;
		}
		inline void reset(Camera & cam) {
			player.health = 100;
			cam.Position = player.getRespawnLocation();
		}
		inline irrklang::ISoundEngine * getSoundEngine() {
			return soundEngine;
		}
		virtual void renderScene(Shader shader, bool depthPass, Camera & cam, float dt, int graphics, glm::mat4 view = glm::mat4(), glm::mat4 projection = glm::mat4()) = 0;
	};
	class Scene1 : public Scene{
	private:
		float lastYaw;
		float lastPitch;
		Model castle, street, house2, village, tower, cathedral, well, forum, bow, arrow, house, knight;
		Quad sun;
		Quad ground, wall;
		glm::mat4 _model;
		int overflow = 0;
		std::vector<BoundingBox> hitBoxes;
		bool firstPass = true;
		int err = 1;
		unsigned int houseNormal; 
//		Debug * test;
		Shader * markerShader;
		Marker * marker;
		bool canPlay = true;
		double lastInjury = 0;
		unsigned int knightTexture;
		Camera cam;
	protected:
/*		void doArrows() {
			clock_t time = clock();
			if (newArrow.getNotifications()) {
				Arrow arr = Arrow(cam, arrow, player.pid, getTime());
				client::getNewArrowId(arr.getId());
				arrows->addElement(arr);
				newArrow.handleNotification();
			}
			for (auto it = arrows->begin(); it != arrows->end(); it++) {
				client::sendExistingArrow((*it).toPacket());
			}
			serverArrows.erase(serverArrows.begin(), serverArrows.end());
			client::getArrows(serverArrows);
			if (!done) printf("Arrow thread takes: %f, \n", (((double)clock() - (double)time) / CLOCKS_PER_SEC));

		}
		void doPlayers() {
			clock_t time = clock();
			client::sendPos(user);
			players.erase(players.begin(), players.end());
			client::getPos(players);
			if (!done) printf("Player thread takes %f \n", (clock() - time) / (double)CLOCKS_PER_SEC);
		}
		void doInfo() {
			if (deathCond.getNotifications()) {
				client::notifyDeath(deathArrow.toPacket());
				deathCond.handleNotification();
			}
			if (UNLOCKED(killsLock)) {
				killsLock = true;
				client::getKills(kills);
				killsCond.notify();
				killsLock = false;
			}
			if (UNLOCKED(leaderLock)) {
				leaderLock = true;
				client::getLeader(leaderKills, leaderName);
				leaderCond.notify();
				leaderLock = false;
			}
			if (UNLOCKED(nameLock)) {
				nameLock = true;
				char * _name;
				client::getName(&_name);
				username = _name;
				delete[] _name;
				nameLock = false;

			}
			if (commandCond.getNotifications() && commandLock == false) {
				commandLock = true;
				client::sendCommand(newCommand);
				commandCond.handleNotification();
				commandLock = false;
			}
		}*/

	public:
		~Scene1() {
			delete marker;
		}
		Scene1(char * username, char * ip) {
			name = "scene1";
			castle.init("Assets/Castle X6.obj");
			house.init("Assets/house_obj.obj");
			street.init("Assets/medstreet.obj");
			sun.init("Assets/sun.png");
			forum.init("Assets/GothicSquare.3ds");
			ground.init("Assets/Grass.jpg");
			house2.init("Assets/Medieval-House.obj");
			village.init("Assets/Village.obj");
			tower.init("Assets/saintriqT3DS.obj");
			cathedral.init("Assets/kosciol.3ds");
			well.init("Assets/Well/well.obj");								
			wall.init("Assets/brickwall.jpg");
			bow.init("Assets/Merciless Crossbow.obj");
			knight.init("Assets/chevalier.obj");
			arrow.init("Assets/Arrow.fbx");
			wall.addTexture("Assets/brickwall_normal.jpg", "normal");
			arrows = new Ringbuffer<Arrow>(12);
			houseNormal = loadTexture("Assets/house_normal.tga");
			knightTexture = loadTexture("Assets/chevalier.bmp");
			markerShader = new Shader("simple.glslbin", "simple.fragbin", true);
			marker = new Marker(markerShader, 1);
			client::startup(ip);
			if (client::sendName(username) == 0) {
				connected = true;
				printf("Client connected \n");
//				client::getPid(player.pid);
				std::thread t(&Scene::conversationThread, this);
				t.detach();
			}
			else {
				connected = false;
			}
//			client::getPid(player.pid);
//			loadSound("Assets/sounds/arrowShoot.wav", &soundList.list[soundList.s_arrowShoot], soundList.sizes[soundList.s_arrowShoot]);
//			hitBoxes.push_back(BoundingBox(glm::vec3(-28, -3, -0.8), glm::vec3(-22, 1, 2.2)));
#pragma region defineBoxes
			hitBoxes.push_back(BoundingBox(glm::vec3(-11.5, -3, -22), glm::vec3(-8.5, 1, -17.8)));
			hitBoxes.push_back(BoundingBox(glm::vec3(-8.5, -2, -22), glm::vec3(-7, 1, -17.8)));
			hitBoxes.push_back(BoundingBox(glm::vec3(-15.4, -3, -30.9), glm::vec3(-6.3, 1, -28.7)));
			hitBoxes.push_back(BoundingBox(glm::vec3(-6.5, -3, -31.3), glm::vec3(-3.4, 1, -28.5)));
			hitBoxes.push_back(BoundingBox(glm::vec3(-6.5, -2, -32.45), glm::vec3(-4.3, 1, -31.27)));
			hitBoxes.push_back(BoundingBox(glm::vec3(-1.6, -3, -30.7), glm::vec3(2.0, -1, -29.1)));
			hitBoxes.push_back(BoundingBox(glm::vec3(0.53, -3, -31.19), glm::vec3(1.45, -2, -30.66)));
			hitBoxes.push_back(BoundingBox(glm::vec3(-1.14, -3, -29.28), glm::vec3(0.79, -1, -28.49)));
			hitBoxes.push_back(BoundingBox(glm::vec3(-25.52, -3, -30.96), glm::vec3(-16.41, 1, -28.86)));
			hitBoxes.push_back(BoundingBox(glm::vec3(-31.7, -3, -30.9), glm::vec3(-27.9, 1, -28.14)));
			hitBoxes.push_back(BoundingBox(glm::vec3(8.83, -3, -23.63), glm::vec3(10.89, 1, -14.51)));
			hitBoxes.push_back(BoundingBox(glm::vec3(-15.4, -3, -30.9), glm::vec3(-6.3, 1, -28.7)));
			hitBoxes.push_back(BoundingBox(glm::vec3(13.21, -3, -31.29), glm::vec3(16.58, 1, -28.8)));
			hitBoxes.push_back(BoundingBox(glm::vec3(13.21, -2, -32.56), glm::vec3(15.5, 1, -31.29)));
			hitBoxes.push_back(BoundingBox(glm::vec3(13, -3, -21.32), glm::vec3(16.57, 1, -18.7)));
			hitBoxes.push_back(BoundingBox(glm::vec3(18.04, -3, -31.29), glm::vec3(21.65, 1, -28.6)));
			hitBoxes.push_back(BoundingBox(glm::vec3(18.04, -2, -32.56), glm::vec3(20.63, 1, -31.29)));
			hitBoxes.push_back(BoundingBox(glm::vec3(13.2, -2, -22.56), glm::vec3(15.63, 1, -21.3)));
			hitBoxes.push_back(BoundingBox(glm::vec3(8.16, -3, -31.32), glm::vec3(11.72, 1, -28.8)));
			hitBoxes.push_back(BoundingBox(glm::vec3(8.16, -2, -32.55), glm::vec3(10.9, 1, -32.55)));
			hitBoxes.push_back(BoundingBox(glm::vec3(-15.4, -3, -30.9), glm::vec3(-6.3, 1, -28.7)));
			hitBoxes.push_back(BoundingBox(glm::vec3(19.1, -3, -23.6), glm::vec3(20.96, 1, -14.1)));
			hitBoxes.push_back(BoundingBox(glm::vec3(18.77, -3, -11.06), glm::vec3(21.28, 1, -8.5)));
			hitBoxes.push_back(BoundingBox(glm::vec3(38.3, -3, -11.12), glm::vec3(41.99, 1, -8.7)));
			hitBoxes.push_back(BoundingBox(glm::vec3(38.3, -2, -12.84), glm::vec3(40.75, 1, -11.33)));
			hitBoxes.push_back(BoundingBox(glm::vec3(34.4, -3, -10.8), glm::vec3(37.91, 1, -7.5)));
			hitBoxes.push_back(BoundingBox(glm::vec3(25.026, -3, -21.25), glm::vec3(33.97, 1, -15.7)));
			hitBoxes.push_back(BoundingBox(glm::vec3(35.5, -3, -7.3), glm::vec3(37.8, -1, -5.1)));
			hitBoxes.push_back(BoundingBox(glm::vec3(34.85, -3, -4.6), glm::vec3(37.95, 1, -1.2)));
			hitBoxes.push_back(BoundingBox(glm::vec3(39, -3, -3.38), glm::vec3(41.4, 1, 5.8)));
			hitBoxes.push_back(BoundingBox(glm::vec3(21.82, -3, -1.69), glm::vec3(27.03, 1, 1.73)));
			hitBoxes.push_back(BoundingBox(glm::vec3(23.14, -3, 4.29), glm::vec3(27.19, -1, 7.3)));
			hitBoxes.push_back(BoundingBox(glm::vec3(25.5, -3, 3.78), glm::vec3(26.5, -2, 4.43)));
			hitBoxes.push_back(BoundingBox(glm::vec3(31.3, -3, 16.2), glm::vec3(35.14, -1, 18)));
			hitBoxes.push_back(BoundingBox(glm::vec3(23.24, -3, 15.9), glm::vec3(27.2, -1, 18.8)));
			hitBoxes.push_back(BoundingBox(glm::vec3(24.53, -3, 19.09), glm::vec3(33.68, 1, 21.27)));
			hitBoxes.push_back(BoundingBox(glm::vec3(38.68, -3, 18.36), glm::vec3(41.32, 1, 21.9)));
			hitBoxes.push_back(BoundingBox(glm::vec3(41.46, -2, 18.5), glm::vec3(42.73, 1, 20.8)));
			hitBoxes.push_back(BoundingBox(glm::vec3(23.09, -3, 12.19), glm::vec3(20.22, 1, 15.66)));
			hitBoxes.push_back(BoundingBox(glm::vec3(19.408, -3, 12.5), glm::vec3(17.79, 1, 14.7)));
			hitBoxes.push_back(BoundingBox(glm::vec3(16.95, -3, 12.09), glm::vec3(14.52, 1, 15.92)));
			hitBoxes.push_back(BoundingBox(glm::vec3(10.95, -3, 8.9), glm::vec3(9.05, 1, 18.4)));
			hitBoxes.push_back(BoundingBox(glm::vec3(13.68, -3, 18.8), glm::vec3(4.5, 1, 20.9)));
			hitBoxes.push_back(BoundingBox(glm::vec3(29.19, -3, 28.7), glm::vec3(31.81, -1, 32.7)));
			hitBoxes.push_back(BoundingBox(glm::vec3(23.56, -3, 28.9), glm::vec3(27.11, -1, 32)));
			hitBoxes.push_back(BoundingBox(glm::vec3(14.48, -3, 22), glm::vec3(17.03, 0, 25.6)));
			hitBoxes.push_back(BoundingBox(glm::vec3(17.97, -3, 22.4), glm::vec3(19.78, -1, 22.0)));
			hitBoxes.push_back(BoundingBox(glm::vec3(20, -3, 22.4), glm::vec3(23, -1, 24.2)));
			hitBoxes.push_back(BoundingBox(glm::vec3(19.9, -3, 12), glm::vec3(23.1, 1, 15.5)));
			hitBoxes.push_back(BoundingBox(glm::vec3(17.9, -3, 12.3), glm::vec3(19.6, -1, 14.5)));
			hitBoxes.push_back(BoundingBox(glm::vec3(14.5, -3, 12.2), glm::vec3(17.1, 1, 15.7)));
			hitBoxes.push_back(BoundingBox(glm::vec3(4.4, -3, 19.06), glm::vec3(13.6, 1, 21.4)));
			hitBoxes.push_back(BoundingBox(glm::vec3(9.03, -3, 9.3), glm::vec3(10.94, 1, 18.57)));
			hitBoxes.push_back(BoundingBox(glm::vec3(20.25, -3, 32.2), glm::vec3(22.83, 1, 34.9)));
			std::ifstream input;
			char file[256];
			sprintf_s(file, 256, "%sMap.db", name);
			input.open(file, std::ios::binary);
			if (input.is_open()) {
				while (!input.eof()) {
					glm::vec3 min;
					glm::vec3 max;
					input.read((char*)&min.x, sizeof(float));
					input.read((char*)&min.y, sizeof(float));					
					input.read((char*)&min.z, sizeof(float));
					input.read((char*)&max.x, sizeof(float));
					input.read((char*)&max.y, sizeof(float));
					if (max.y - min.y < 0.3) max.y = 1;
					input.read((char*)&max.z, sizeof(float));
					if (min.y > -3 && min.y < -2) min.y = -3;
					hitBoxes.push_back(BoundingBox(min, max));
					printf("Read box %f, %f, %f to %f, %f, %f\n", min.x, min.y, min.z, max.x, max.y, max.z);
				}
				input.close();

			}
			else {
				printf("Not open \n");
				char msg[256];
				sprintf_s(msg, 256, "%s Map data not found! (%sMap.db) You cannot play without this!", name);
				MessageBox(NULL, msg, "Loading Error!",  MB_OK | MB_ICONERROR);
				canPlay = false;
			}
#pragma endregion

		}
		void addArrow(Camera & cam) {
			soundEngine->play2D("Assets/sounds/arrowShoot.wav");
//			newArrow.notify();
			Arrow arr = Arrow(cam, arrow, player.pid, getTime());
//			if(client::getNewArrowId(arr.getId()) != 0) arr.getId() = 0;
	//		arrows->addElement(arr);
			arr.newShot = true;
			if (connected) {
				std::lock_guard<std::mutex> guard(arrowMutex);
				arr.getId() = arrowIds.front();
				arrowIds.pop();
			}
			else arr.getId() = 0;
			printf("New arrow with id %d \n", arr.getId());
			if (UNLOCKED(arrowLock)) {
				arrowLock = true;
				printf("Added to ringbuffer \n");
				arrows->addElement(arr);
				arrowLock = false;
			}
			else {
				backupLock = true;
				printf("Pushed onto arr to be added \n");
				arrowsToBeAdded.push(arr.toPacket());
				allArrows.push_back(arr);
				backupCond.notify();
				backupLock = false;
			}			
		}
		void renderScene(Shader shader, bool depthPass, Camera & cam, float dt, int graphics, glm::mat4 view = glm::mat4(), glm::mat4 projection = glm::mat4()) {			
			if (!canPlay) return;
			clock_t startTime = clock();
/*			if (!depthPass) { timing(); }
			std::thread * t = NULL;
			if (!depthPass) {
				this->cam = cam;
				t = new std::thread(&Scene1::doArrows, this);
			}
			clock_t time = clock();*/
			glm::mat4 model;
			model = glm::translate(model, glm::vec3(0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			castle.Draw(shader);
#pragma region DrawHouse1
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-25, -3.0, 0));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
//			shader.setBool("useNormal", true);
//			glActiveTexture(GL_TEXTURE2);
//			glBindTexture(GL_TEXTURE_2D, houseNormal);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-25, -3.0, 5));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(25, -3.0, 5));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-25, -3.0, -5));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(30, -3.0, 30));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-30, -3.0, 5));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(25, -3.0, 30));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(25, -3.0, 17));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(10.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(0, -3.0, -30));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-30, -3.0, -30));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-10, -3.0, 5));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(10.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(33, -3.0, 17));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house.Draw(shader); 
//			shader.setBool("useNormal", false);
#pragma endregion
#pragma region DrawVillage
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(0, -3.2, 30));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			village.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(10, -3.2, 30));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			village.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(20, -3.2, 30));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			village.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(20, -3.2, 20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			village.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(20, -3.2, 10));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			village.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(40, -3.2, -5));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			village.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-18, -3.2, -20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			village.Draw(shader);
#pragma endregion
#pragma region DrawStreet
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(10, -3.2, 20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(30, -3.2, 20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(10, -3.2, -20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(20, -3.2, -20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(30, -3.2, -20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-20, -3.2, 20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(40, -3.2, 0));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(10, -3.2, 13));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-10, -3.2, -30));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-20, -3.2, -30));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(1.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			street.Draw(shader);
#pragma endregion
#pragma region DrawHouse2
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(10, -3.2, -30));
			model = glm::scale(model, glm::vec3(.5f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house2.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(15, -3.2, -30));
			model = glm::scale(model, glm::vec3(.5f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house2.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(20, -3.2, -30));
			model = glm::scale(model, glm::vec3(.5f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house2.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-10, -3.2, -20));
			model = glm::scale(model, glm::vec3(.5f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house2.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(40, -3.2, 20));
			model = glm::scale(model, glm::vec3(.5f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house2.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-5, -3.2, -30));
			model = glm::scale(model, glm::vec3(.5f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house2.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(15, -3.2, -20));
			model = glm::scale(model, glm::vec3(.5f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house2.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(40, -3.2, -10));
			model = glm::scale(model, glm::vec3(.5f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			house2.Draw(shader);
#pragma endregion
#pragma region DrawMisc
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(25, -1, 0));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0, 0.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			cathedral.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-10, -3.2, 20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0, 0.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			tower.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(20, -3.2, -10));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			tower.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(0, -3.0, 20));
			model = glm::scale(model, glm::vec3(.009f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			well.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-30, -4, 20));
			model = glm::scale(model, glm::vec3(.029f));
			model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0, 0.0, 0.0));
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 0.0, 1.0));
			SHADER_SET_MAT4(shader, "model", model);("model", model);
			forum.Draw(shader);
//			shader.setBool("useNormal", false);
//			shader.setBool("useSpecular", false);
#pragma endregion
#pragma region DrawRects
			glm::mat4 sunModel;
			sunModel = glm::mat4();
			sunModel = glm::translate(sunModel, glm::vec3(0.0, -2.0, 0.0));
			sunModel = glm::scale(sunModel, glm::vec3(2));
//			sunModel = glm::rotate(sunModel, glm::radians(90.0f), glm::vec3(0.0, 0.0, 0.0));
			SHADER_SET_MAT4(shader, "model", sunModel);
			ground.draw();
			sunModel = glm::mat4();
			sunModel = glm::translate(sunModel, glm::vec3(0.0, 6.9, 0.0));
			sunModel = glm::scale(sunModel, glm::vec3(20));
			SHADER_SET_MAT4(shader, "model", sunModel);
			ground.draw();
/*			sunModel = glm::mat4();
			sunModel = glm::rotate(sunModel, glm::radians(90.f), glm::vec3(1, 0, 0));
			sunModel = glm::translate(sunModel, glm::vec3(0, 50, -50));
			sunModel = glm::scale(sunModel, glm::vec3(2));
			SHADER_SET_MAT4(shader, "model", model);("model", sunModel);
			sun.draw();*/
/*			if (!depthPass) {
				if (!done) printf("First section takes %f \n", (clock() - time) / (double)CLOCKS_PER_SEC);
				t->join();
				delete t;
				t = new std::thread(&Scene1::doPlayers, this);
				time = clock();
			}*/
#pragma endregion

#pragma region DrawBow/Arrows/Players
			if (!depthPass) {
				model = glm::mat4();
				int xfactor = (cam.Front.z > 0) ? -1 : 1;
				int zfactor = (cam.Front.x < 0) ? -1 : 1;
				model = glm::translate(model, (cam.Position - glm::vec3(0, 0.2, 0)) + glm::vec3(xfactor * 0.05, 0, zfactor * 0.05));
				model = glm::scale(model, glm::vec3(0.009));
				if (!cam.look) {
					model = glm::rotate(model, glm::radians(-cam.Yaw), glm::vec3(0.0, 1.0, 0.0));
					glm::vec3 axis = glm::normalize(glm::cross(cam.Front, glm::vec3(0, 1, 0)));
					model = glm::rotate(model, glm::radians(cam.Pitch), glm::vec3(0, 0, 1));
					lastYaw = -cam.Yaw;
					lastPitch = cam.Pitch;
				}
				else {
					model = glm::rotate(model, glm::radians(lastYaw), glm::vec3(0.0, 1.0, 0.0));
					model = glm::rotate(model, glm::radians(lastPitch), glm::vec3(0.0, 0.0, 1.0));
				}
				model = glm::rotate(model, glm::radians(278.0f), glm::vec3(0.0, 1.0, 0.0));
				model = glm::rotate(model, glm::radians(-6.3f), glm::vec3(0.0, 0.0, 1.0));
				SHADER_SET_MAT4(shader, "model", model);("model", model);
				bow.Draw(shader);
				if (UNLOCKED(arrowLock)) {
					arrowLock = true;
//					printf("Handling arrows size: %d \n", arrows->getSize());
					for (auto it = arrows->begin(); it != arrows->end(); it++) {
						if ((*it).newShot) (*it).newShot = false;
//						client::sendExistingArrow((*it).toPacket());
						(*it).draw(shader, dt, depthPass, getTime());
						glm::vec3 point = (*it).getPoint();
						if ((*it).isAlive()) {
							if (point.y < -2.92) {
								(*it).collide();
								soundEngine->play3D("Assets/sounds/arrow-impact.wav", vectorAdapter(point - cam.Position));
							}
							else {
								for (int i = 0; i < hitBoxes.size(); i++) {
									if (hitBoxes[i][point]) {
										(*it).collide();
										soundEngine->play3D("Assets/sounds/arrow-impact.wav", vectorAdapter(point - cam.Position));
										break;
									}
								}
							}
						}
					}
					arrowLock = false;
				}
//				serverArrows.erase(serverArrows.begin(), serverArrows.end());
				if (connected == false/*client::getArrows(serverArrows) != 0*/) {
					//offline
					allArrows.erase(allArrows.begin(), allArrows.end());
					for (auto it = arrows->begin(); it != arrows->end(); it++) {
						(*it).setModel(arrow);
						allArrows.push_back((*it));
					}
				}
				else {
					std::vector<Arrow> serverArrows = getServerArrows();
					allArrows.erase(allArrows.begin(), allArrows.end());
//					printf("Size of server arrows: %d \n", serverArrows.size());
					for (int i = 0; i < serverArrows.size(); i++) {
//						printf("Recieving arrow \n");
//						if (allArrows[i].getShooter() == player.pid) continue;
						allArrows.push_back(serverArrows[i]);
						allArrows[i].setModel(arrow);
						if (allArrows[i].newShot && allArrows[i].getShooter() != player.pid) {
							//						soundEngine->play3D("Assets/sounds/arrowShoot.wav", vectorAdapter(point - cam.Position));
						}
						//					printf("Online Arrow pos %f %f %f \n", allArrows[i].getPos().x, allArrows[i].getPos().y, allArrows[i].getPos().z);
					}
				}
				for (auto it = allArrows.begin(); it != allArrows.end(); it++) {
					//				if (graphics == 1 && depthPass == false) {
					(*it).draw(shader, dt, depthPass, getTime());
					//				}else if(!depthPass || graphics != 1) (*it).draw(shader, dt, depthPass);
					glm::vec3 point = (*it).getPoint();
					if ((*it).isAlive() && ((*it).getShooter() != player.pid && (*it).getShooter() != 35000)) {
						if (M_DISTANCE(point, cam.Position) < 0.65 && glfwGetTime() - lastInjury > 1) {
//							client::stopArrow((*it).getId());
							lastInjury = glfwGetTime();
							player.health -= 25;
							player.takeDamage();
							soundEngine->play2D("Assets/sounds/arrow-impact.wav");
							//						(*it).collide();
							if (player.health <= 0) {
//								client::notifyDeath((*it).toPacket());
								died((*it));
							}
						}
					}
				}
				cam.applyForces(dt, hitBoxes);
				unsigned char movement = 0;
				if (cam.move) movement |= GAME_PLAYER_MOVER;
				else if (cam.fastMove) movement |= GAME_PLAYER_FAST_MOVER;
				else if (cam.shouldPlay) movement |= GAME_PLAYER_PLAY;
				position posNow{ cam.Position.x, cam.Position.y, cam.Position.z, cam.Yaw, cam.Pitch, movement };
				if (cam.look) {
					posNow.yaw = cam.b4LookYaw;
				}
				setPos(posNow);
/*				if (!done) printf("Second section takes %f \n", (clock() - time) / (double)CLOCKS_PER_SEC);
				t->join();
				delete t;
				t = new std::thread(&Scene1::doInfo, this);
				time = clock();*/
//				int send = client::sendPos(posNow);
//				if (send != 0 && send != SCK_CLOSED) printf("Error code %i while sending player positions! \n", send);

//				err = client::getPos(players);
//				if (err != 0 && err != SCK_CLOSED) printf("Error code %i while getting player positions! \n", err);
				loadPlayers();
				if (players.size() > 0) {
					int i = 0;
					for (position p : players) {
						glm::mat4 model = glm::mat4();
						model = glm::translate(model, glm::vec3(p.x, p.y - 0.7, p.z));
						model = glm::rotate(model, glm::radians(-p.yaw + 90.f), glm::vec3(0, 1, 0));
						model = glm::scale(model, glm::vec3(.4));
						SHADER_SET_MAT4(shader, "model", model); ("model", model);
						knight.Draw(shader);
						glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D, knightTexture);
						knight.Draw(shader);
						if (p.allied) {
//							printf("Allied \n");
							model = glm::mat4();
							model = glm::translate(model, glm::vec3(p.x, p.y + 0.2, p.z));
							marker->draw(model, projection, view);
							USE_SHADER(shader);
							
						}
						model = glm::mat4();
						model = glm::translate(model, glm::vec3(p.x, p.y - 0.2, p.z));
						model = glm::scale(model, glm::vec3(0.005));
						model = glm::rotate(model, glm::radians(-p.yaw + 278.f), glm::vec3(0.0, 1.0, 0.0));
						model = glm::rotate(model, glm::radians(-6.3f), glm::vec3(0.0, 0.0, 1.0));
						SHADER_SET_MAT4(shader, "model", model);("model", model);
						bow.Draw(shader);
						if (p.movement) {
							if (playerMoves.size() <= i) playerMoves.push_back(0);
							if (p.movement & GAME_PLAYER_MOVER) {
								if (glfwGetTime() - playerMoves[i] > 0.5) {
									playerMoves[i] = glfwGetTime();
									soundEngine->play3D("Assets/sounds/grass-step.wav", vectorAdapter(glm::vec3(p.x, p.y, p.z) - cam.Position));
								}
							}
							else if (p.movement & GAME_PLAYER_FAST_MOVER) {
								if (glfwGetTime() - playerMoves[i] > 0.4) {
									playerMoves[i] = glfwGetTime();
									soundEngine->play3D("Assets/sounds/grass-step.wav", vectorAdapter(glm::vec3(p.x, p.y, p.z) - cam.Position));
								}
							}
							else if (p.movement & GAME_PLAYER_PLAY) {
								soundEngine->play3D("Assets/sounds/grass-step.wav", vectorAdapter(glm::vec3(p.x, p.y, p.z) - cam.Position));
							}
						}
						i++;
					}
					//				}
				}
				if (player.health <= 0) {
					soundEngine->play2D("Assets/sounds/die.wav");
					cam.Position = player.getRespawnLocation();
					player.health = 100;
				}
				if (cam.shouldPlay) {
					soundEngine->play2D("Assets/sounds/grass-step.wav");
				}
				else if (cam.move) {
					if (glfwGetTime() - cam.moveTime > 0.5) {
						cam.moveTime = glfwGetTime();
						soundEngine->play2D("Assets/sounds/grass-step.wav");
					}
				}
				else if (cam.fastMove) {
					if (glfwGetTime() - cam.moveTime > 0.4) {
						cam.moveTime = glfwGetTime();
						soundEngine->play2D("Assets/sounds/grass-step.wav");
					}
				}
/*				if (!done) printf("Last section takes %f \n", (clock() - time) / (double)CLOCKS_PER_SEC);
				done = true;
				t->join();
				delete t;*/
				if (!depthPass && !done) printf("Scene took: %f \n", (clock() - startTime) / (double)CLOCKS_PER_SEC);
				done = true;
			}
			else if (graphics == 2) {
				for (auto it = allArrows.begin(); it != allArrows.end(); it++) {
					(*it).draw(shader, dt, depthPass | graphics, getTime());
				}
				model = glm::mat4();
				int xfactor = (cam.Front.z > 0) ? -1 : 1;
				int zfactor = (cam.Front.x < 0) ? -1 : 1;
				model = glm::translate(model, (cam.Position - glm::vec3(0, 0.2, 0)) + glm::vec3(xfactor * 0.05, 0, zfactor * 0.05));
				model = glm::scale(model, glm::vec3(0.009));
				if (!cam.look) {
					model = glm::rotate(model, glm::radians(-cam.Yaw), glm::vec3(0.0, 1.0, 0.0));
					glm::vec3 axis = glm::normalize(glm::cross(cam.Front, glm::vec3(0, 1, 0)));
					model = glm::rotate(model, glm::radians(cam.Pitch), glm::vec3(0, 0, 1));
					lastYaw = -cam.Yaw;
					lastPitch = cam.Pitch;
				}
				else {
					model = glm::rotate(model, glm::radians(lastYaw), glm::vec3(0.0, 1.0, 0.0));
					model = glm::rotate(model, glm::radians(lastPitch), glm::vec3(0.0, 0.0, 1.0));
				}
				model = glm::rotate(model, glm::radians(278.0f), glm::vec3(0.0, 1.0, 0.0));
				model = glm::rotate(model, glm::radians(-6.3f), glm::vec3(0.0, 0.0, 1.0));
				SHADER_SET_MAT4(shader, "model", model);
				bow.Draw(shader);

				for (position p : players) {
					glm::mat4 model = glm::mat4();
					model = glm::translate(model, glm::vec3(p.x, p.y - .7, p.z));
					model = glm::rotate(model, glm::radians(-p.yaw + 90.f), glm::vec3(0, 1, 0));
					model = glm::scale(model, glm::vec3(.4));
					SHADER_SET_MAT4(shader, "model", model);
					knight.Draw(shader);
					model = glm::mat4();
					model = glm::translate(model, glm::vec3(p.x, p.y - 0.1, p.z));
					model = glm::scale(model, glm::vec3(0.005));
					model = glm::rotate(model, glm::radians(-p.yaw + 278.f), glm::vec3(0.0, 1.0, 0.0));
					model = glm::rotate(model, glm::radians(-6.3f), glm::vec3(0.0, 0.0, 1.0));
					SHADER_SET_MAT4(shader, "model", model);
					bow.Draw(shader);
				}
				glm::mat4 model = glm::mat4();
				model = glm::translate(model, cam.Position - glm::vec3(0, .7, 0));
				float theta = cam.look ? cam.b4LookYaw : cam.Yaw;
				model = glm::rotate(model, glm::radians(-theta + 90.f), glm::vec3(0, 1, 0));
				model = glm::scale(model, glm::vec3(.4));
				SHADER_SET_MAT4(shader, "model", model);
				knight.Draw(shader); 

			}
//			printf("Pos: %f, %f, %f \n", cam.Position.x, cam.Position.y, cam.Position.z);
			
#pragma endregion



		}
	};
	Scene * sceneFactory(int scene, char * username, char * ip) {
		Scene * map;
		switch (scene) {
		case 0:
			map = new Scene1(username, ip);
			break;
		default:
			map = new Scene1(username, ip);
			break;
		}
		return map;
	}
}