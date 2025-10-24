#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"
#include "Map.h"

Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
}

Player::~Player() {
}

bool Player::Awake() {

	//L03: TODO 2: Initialize Player parameters
	position = Vector2D(96, 96);
	return true;
}

bool Player::Start() {

	// load
	std::unordered_map<int, std::string> aliases = { {0,"idle"},{6,"move"},{12,"jump"} };
	anims.LoadFromTSX("Assets/Textures/MikuSpriteSheet.tsx", aliases);
	anims.SetCurrent("idle");

	//L03: TODO 2: Initialize Player parameters
	texture = Engine::GetInstance().textures->Load("Assets/Textures/MikuSpriteSheet.png");

	// L08 TODO 5: Add physics to the player - initialize physics body
	//Engine::GetInstance().textures->GetSize(texture, texW, texH);
	texW = 32;
	texH = 32;
	pbody = Engine::GetInstance().physics->CreateCircle((int)position.getX(), (int)position.getY(), texW / 2, bodyType::DYNAMIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	//initialize audio effect
	pickCoinFxId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/coin-collision-sound-342335.wav");

	return true;
}

bool Player::Update(float dt)
{
	GodMode();

	if (godMode == false) {
		GetPhysicsValues();
		Move();
		Jump();
		ApplyPhysics();
	}
	Draw(dt);

	return true;
}

void Player::GetPhysicsValues() {
	// Read current velocity
	velocity = Engine::GetInstance().physics->GetLinearVelocity(pbody);
	velocity = { 0, velocity.y }; // Reset horizontal velocity by default, this way the player stops when no key is pressed
}

void Player::Move() {
	
	// Move left/right
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
		velocity.x = -speed;
		anims.SetCurrent("move");
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
		velocity.x = speed;
		anims.SetCurrent("move");
	}
}

void Player::Jump() {
	// This function can be used for more complex jump logic if needed
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
		if (!isJumping) {
			Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -jumpForce, true);
			anims.SetCurrent("jump");
			isJumping = true;
			doubleJump = true;
		}
		// Second jump (in air)
		else if (isJumping && doubleJump) {
			Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -jumpForce, true);
			anims.SetCurrent("jump");
			doubleJump = false;
		}
	}
}

void Player::ApplyPhysics() {
	// Preserve vertical speed while jumping
	if (isJumping == true) {
		velocity.y = Engine::GetInstance().physics->GetYVelocity(pbody);
	}

	// Apply velocity via helper
	Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
}

void Player::Draw(float dt) {

	anims.Update(dt);
	const SDL_Rect& animFrame = anims.GetCurrentFrame();

	// Update render position using your PhysBody helper
	int x, y;
	pbody->GetPosition(x, y);
	position.setX((float)x);
	position.setY((float)y);
	Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2, &animFrame);

	//L10: TODO 7: Center the camera on the player
	float limitLeft = Engine::GetInstance().render->camera.w / 4;
	float limitRight = Engine::GetInstance().map->GetMapSizeInPixels().getX() - Engine::GetInstance().render->camera.w * 3 / 4;
	float limitUp = Engine::GetInstance().render->camera.h / 4;
	float limitDown = Engine::GetInstance().map->GetMapSizeInPixels().getY() - Engine::GetInstance().render->camera.h * 3 / 4;
	if (position.getX() - limitLeft > 0 && position.getX() < limitRight) {
		Engine::GetInstance().render->camera.x = -position.getX() + Engine::GetInstance().render->camera.w / 4;
	}
	if (position.getY() - limitUp > 0 && position.getY() < limitDown) {
		Engine::GetInstance().render->camera.y = -position.getY() + Engine::GetInstance().render->camera.h / 4;
	}
}

void Player::GodMode()
{
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F10) == KEY_DOWN) {
		//God Mode toggle
		godMode = !godMode;
		if (godMode == true) {
			b2Body_SetGravityScale(pbody->body, 0.0f);
			LOG("God Mode enabled");
		}
		if (godMode == false) {
			b2Body_SetGravityScale(pbody->body, 1.0f );
			LOG("God Mode disabled");
		}
	}
	if (godMode) {
		//Starts with no movement
		b2Vec2 velocity = { 0.0f, 0.0f };

		//Assigns a certain value to the local variable velocity according to the key pressed
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
			velocity.x = -speed;
			anims.SetCurrent("move");
		}
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
			velocity.x = speed;
			anims.SetCurrent("move");
		}
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
			velocity.y = -speed;
			anims.SetCurrent("move");
		}
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
			velocity.y = speed;
			anims.SetCurrent("move");
		}
		else {
			anims.SetCurrent("idle");
		}

		//Applies the velocity to the player
		b2Body_SetLinearVelocity(pbody->body, velocity);
	}
}

bool Player::CleanUp()
{
	LOG("Cleanup player");
	Engine::GetInstance().textures->UnLoad(texture);
	return true;
}

// L08 TODO 6: Define OnCollision function for the player. 
void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("Collision PLATFORM");
		//reset the jump flag when touching the ground
		isJumping = false;
		doubleJump = false;
		anims.SetCurrent("idle");
		LOG("Animation set to idle");
		break;
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		Engine::GetInstance().audio->PlayFx(pickCoinFxId);
		physB->listener->Destroy();
		break;
	case ColliderType::VOID:
		LOG("Collision VOID");
		gameOver = true;
		break;
	default:
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("End Collision PLATFORM");
		break;
	case ColliderType::ITEM:
		LOG("End Collision ITEM");
		break;
	case ColliderType::UNKNOWN:
		LOG("End Collision UNKNOWN");
		break;
	default:
		break;
	}
}