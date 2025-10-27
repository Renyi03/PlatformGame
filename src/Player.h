#pragma once

#include "Entity.h"
#include "Animation.h"
#include <box2d/box2d.h>
#include <SDL3/SDL.h>
#include "Player.h"

struct SDL_Texture;

class Player : public Entity
{
public:

	Player();
	
	virtual ~Player();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	// L08 TODO 6: Define OnCollision function for the player. 
	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

private:

	void GetPhysicsValues();
	void Move();
	void Jump();
	void ApplyPhysics();
	void Draw(float dt);
	void GodMode();

public:

	//Declare player parameters
	float speed = 4.0f;
	SDL_Texture* texture = NULL;

	int texW, texH;

	//Audio fx
	int pickCoinFxId;

	// L08 TODO 5: Add physics to the player - declare a Physics body
	PhysBody* pbody;
	float jumpForce = 2.5f; // The force to apply when jumping
	bool isJumping = false; // Flag to check if the player is currently jumping
	bool doubleJump = false; // Flag to check if the player has consumed the double jump
	bool godMode = false; // Flag to check if the player has activated God Mode
	bool gameOver = false; // Flag to check if the player has died
	SDL_FlipMode facingDirection = SDL_FLIP_NONE;
private: 
	b2Vec2 velocity;
	AnimationSet anims;
};