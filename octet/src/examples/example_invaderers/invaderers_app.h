#include "tinyxml/tinyxml.h"
#include "sprite.h"
#include <cstdlib>
#include <ctime>

namespace octet {

	class invaderers_app : public octet::app {

		mat4t cameraToWorld;
		texture_shader texture_shader_;
		alternative_shader alternative_shader_;

		enum {
			num_sound_sources = 8,
			num_rows = 5,
			num_cols = 10,
			num_missiles = 2,
			num_bombs = 3,
			num_borders = 4,
			num_enemys = num_rows * num_cols,

			// sprite definitions
			player_sprite = 0,
			castle_sprite,
			game_over_sprite,

			first_enemy_sprite,
			last_enemy_sprite = first_enemy_sprite + num_enemys - 1,

			first_missile_sprite,
			last_missile_sprite = first_missile_sprite + num_missiles - 1,

			first_bomb_sprite,
			last_bomb_sprite = first_bomb_sprite + num_bombs - 1,

			first_border_sprite,
			last_border_sprite = first_border_sprite + num_borders - 1,

			num_sprites,

		};

		// timers for missiles and bombs
		int missiles_disabled;
		int bombs_disabled;

		// accounting for bad guys
		int live_enemys;
		int num_lives;

		// game state
		bool game_over;
		int score;

		// speed of enemy
		float invader_velocity;

		// sounds
		ALuint whoosh;
		ALuint bang;
		ALuint death;
		unsigned cur_source;
		ALuint sources[num_sound_sources];

		// big array of sprites
		sprite sprites[num_sprites];

		// random number generator
		class random randomizer;

		// a texture for our text
		GLuint font_texture;

		// information for our text
		bitmap_font font;

		//last direction travelled by player
		string lastDirection;

		ALuint get_sound_source() { return sources[cur_source++ % num_sound_sources]; }

		void dodge() {

			const float player_speed = 0.05f;

			if (is_key_going_down(' ')) {

				if (lastDirection == "up") {
					sprites[player_sprite].translate(0, -player_speed * 15);
					if (sprites[player_sprite].collides_with(sprites[first_border_sprite + 2])) {
						sprites[player_sprite].translate(0, +player_speed);
					}
				}
				else if (lastDirection == "down") {
					sprites[player_sprite].translate(0, +player_speed * 15);
					if (sprites[player_sprite].collides_with(sprites[first_border_sprite + 3])) {
						sprites[player_sprite].translate(0, -player_speed);
					}
				}

			}
		}

		// use the keyboard to move the player
		void move_player() {
			const float player_speed = 0.05f;
			// left and right arrows
			if (is_key_down(key_up)) {
				lastDirection = "up";
				sprites[player_sprite].translate(0, +player_speed);
				if (sprites[player_sprite].collides_with(sprites[first_border_sprite + 2])) {
					sprites[player_sprite].translate(0, -player_speed);
				}
			}
			else if (is_key_down(key_down)) {
				lastDirection = "down";
				sprites[player_sprite].translate(0, -player_speed);
				if (sprites[player_sprite].collides_with(sprites[first_border_sprite + 3])) {
					sprites[player_sprite].translate(0, +player_speed);
				}
			}
		}

		// called when we hit an enemy
		void on_hit_enemy() {
			ALuint source = get_sound_source();
			alSourcei(source, AL_BUFFER, bang);
			alSourcePlay(source);

			live_enemys--;
			score++;
			if (live_enemys == 4) {
				invader_velocity *= 4;
			}
			else if (live_enemys == 0) {
				game_over = true;
				sprites[game_over_sprite].translate(-20, 0);
			}
		}

		void fire_bombs() {
			if (bombs_disabled) {
				--bombs_disabled;
			}
			else {
				// find an invaderer
				sprite &castle = sprites[castle_sprite];
				sprite &player = sprites[player_sprite];

				for (int j = randomizer.get(0, num_enemys); j < num_enemys; ++j) {
					sprite &enemy = sprites[first_enemy_sprite + j];
					if (enemy.is_enabled() && enemy.is_adjacent(castle, 0.3f) && enemy.is_onscreen()) {
						// find a bomb
						for (int i = 0; i != num_bombs; ++i) {
							if (!sprites[first_bomb_sprite + i].is_enabled()) {
								sprites[first_bomb_sprite + i].direction = "left";
								sprites[first_bomb_sprite + i].set_relative(enemy, -0.25f, 0);
								sprites[first_bomb_sprite + i].is_enabled() = true;
								bombs_disabled = 15;
								ALuint source = get_sound_source();
								alSourcei(source, AL_BUFFER, whoosh);
								alSourcePlay(source);
								return;
							}
						}
						return;
					}
				}
			}
		}

		void move_bombs() {
			const float bomb_speed = 0.2f;
			for (int i = 0; i != num_bombs; ++i) {
				sprite &bomb = sprites[first_bomb_sprite + i];
				if (bomb.is_enabled()) {
					
					if (bomb.direction == "left") {
						bomb.translate(-bomb_speed, 0);
						
						if (bomb.collides_with(sprites[player_sprite])) {
							bombs_disabled = 15;
							bomb.direction = "right";
							goto next_bomb;
						}

						if (bomb.collides_with(sprites[castle_sprite])) {
							bomb.is_enabled() = false;
							bomb.translate(20, 0);
							bombs_disabled = 50;
							on_hit_castle();
							goto next_bomb;
						}
					}
					else if(bomb.direction == "right") {

						bomb.translate(bomb_speed, 0);

						for (int j = 0; j != num_enemys; ++j) {
							sprite &enemy = sprites[first_enemy_sprite + j];
							if (enemy.is_enabled() && bomb.collides_with(enemy)) {
								enemy.is_enabled() = false;
								enemy.translate(20, 0);
								bomb.is_enabled() = false;
								bomb.translate(20, 0);
								on_hit_enemy();

								goto next_bomb;
							}
						}
					}
					
					if (bomb.collides_with(sprites[first_border_sprite + 2]) || bomb.collides_with(sprites[first_border_sprite + 3])) {
						bomb.is_enabled() = false;
						bomb.translate(20, 0);
					}
				}
			next_bomb:;
			}
		}

		void on_hit_castle() {
			ALuint source = get_sound_source();
			alSourcei(source, AL_BUFFER, bang);
			alSourcePlay(source);

			if (--num_lives == 0) {
				ALuint source = get_sound_source();
				alSourcei(source, AL_BUFFER, death);
				alSourcePlay(source);
				game_over = true;
				sprites[game_over_sprite].translate(-20, 0);
			}
		}

		// move the array of enemies
		void move_enemys(float dx, float dy) {
			for (int j = 0; j != num_enemys; ++j) {
				sprite &enemy = sprites[first_enemy_sprite + j];
				if (enemy.is_enabled()) {
					enemy.translate(dx, dy);
				}
			}
		}

		// check if any invaders hit the sides.
		bool enemys_collide(sprite &border) {
			for (int j = 0; j != num_enemys; ++j) {
				sprite &enemy = sprites[first_enemy_sprite + j];
				if (enemy.is_enabled() && enemy.collides_with(border)) {
					return true;
				}
			}
			return false;
		}

		void draw_text(alternative_shader &shader, float x, float y, float scale, const char *text) {
			mat4t modelToWorld;
			modelToWorld.loadIdentity();
			modelToWorld.translate(x, y, 0);
			modelToWorld.scale(scale, scale, 1);
			mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

			/*mat4t tmp;
			glLoadIdentity();
			glTranslatef(x, y, 0);
			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);
			glScalef(scale, scale, 1);
			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);*/

			enum { max_quads = 32 };
			bitmap_font::vertex vertices[max_quads * 4];
			uint32_t indices[max_quads * 6];
			aabb bb(vec3(0, 0, 0), vec3(256, 256, 0));

			unsigned num_quads = font.build_mesh(bb, vertices, indices, max_quads, text, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, font_texture);

			shader.render(modelToProjection, 0);

			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].x);
			glEnableVertexAttribArray(attribute_pos);
			glVertexAttribPointer(attribute_uv, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].u);
			glEnableVertexAttribArray(attribute_uv);

			glDrawElements(GL_TRIANGLES, num_quads * 6, GL_UNSIGNED_INT, indices);
		}

		/*
		Function to load the XML document object
		
		@return void
		*/
		TiXmlDocument load_xml() {
			TiXmlDocument doc("xml/config.xml");
			GLboolean loadOkay = doc.LoadFile();

			if (loadOkay) {
				return doc;
			}
			else {
				return false;
			}

		}

		/*
		Function to parse the XML document object

		@param vector<string> textures_vector - Holds a reference to all the textures
		@param vector<string> sounds_vector - Holds a reference to all the sounds
		*/
		void load_assets_via_xml(std::vector<string>& textures_vector, std::vector<string>& sounds_vector) {

			
			TiXmlDocument doc = load_xml();
			TiXmlElement *texture, *textures, *assets, *sounds, *sound;
			std::vector<string> config_vec;

			assets = doc.FirstChildElement("assets");

			if (assets)
			{
				textures = assets->FirstChildElement("textures");
				if (textures) {
					texture = textures->FirstChildElement("texture");

					while (texture)
					{
						textures_vector.push_back(texture->GetText());
						texture = texture->NextSiblingElement("texture");
					}
				}
				
				sounds = assets->FirstChildElement("sounds");
				if (sounds) {
					sound = sounds->FirstChildElement("sound");

					while (sound)
					{
						sounds_vector.push_back(sound->GetText());
						sound = sound->NextSiblingElement("sound");
					}
				}
			}

		}

	public:

		// this is called when we construct the class
		invaderers_app(int argc, char **argv) : app(argc, argv), font(512, 256, "assets/big.fnt") {
		}

		// this is called once OpenGL is initialized
		void app_init() {

			srand(static_cast <unsigned> (time(0)));

			//texture and sounds vectors
			std::vector<string> textures;
			std::vector<string> sounds;

			// Load in a config file via XML.
			load_assets_via_xml(textures, sounds);	

			// Iterate through a vector using iterators:
			std::vector<string>::iterator textures_it = textures.begin();
			std::vector<string>::iterator sounds_it = sounds.begin();

			// set up the shader
			texture_shader_.init();
			alternative_shader_.init();

			// set up the matrices with a camera 5 units from the origin
			cameraToWorld.loadIdentity();
			cameraToWorld.translate(0, 0, 3);

			font_texture = resource_dict::get_texture_handle(GL_RGBA, textures_it[0]);

			GLuint player = resource_dict::get_texture_handle(GL_RGBA, textures_it[1]);
			sprites[player_sprite].init(player, -1.25f, 0, 0.05f, 0.5f);

			GLuint GameOver = resource_dict::get_texture_handle(GL_RGBA, textures_it[2]);
			sprites[game_over_sprite].init(GameOver, 20, 0, 3, 1.5f);

			GLuint enemy = resource_dict::get_texture_handle(GL_RGBA, textures_it[3]);
			for (int i = 0; i != num_enemys; ++i) {
				float r3 = -1 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1 - -1)));
				sprites[first_enemy_sprite + i].init(enemy, 3.0f + (i/2), r3, 0.25f, 0.25f);
			}

			// set the border to white for clarity
			GLuint white = resource_dict::get_texture_handle(GL_RGB, textures_it[4]);
			sprites[first_border_sprite + 0].init(white, 0, -3, 6, 3);
			sprites[first_border_sprite + 1].init(white, 0, 3, 6, 3);
			sprites[first_border_sprite + 2].init(white, -3, 0, 0.2f, 6);
			sprites[first_border_sprite + 3].init(white, 3, 0, 0.2f, 6);

			// use the missile texture
			GLuint missile = resource_dict::get_texture_handle(GL_RGBA, textures_it[5]);
			for (int i = 0; i != num_missiles; ++i) {
				// create missiles off-screen
				sprites[first_missile_sprite + i].init(missile, 20, 0, 0.0625f, 0.25f);
				sprites[first_missile_sprite + i].is_enabled() = false;
			}

			// use the bomb texture
			GLuint bomb = resource_dict::get_texture_handle(GL_RGBA, textures_it[6]);
			for (int i = 0; i != num_bombs; ++i) {
				// create bombs off-screen
				sprites[first_bomb_sprite + i].init(bomb, 20, 0, 0.25f, 0.0625f);
				sprites[first_bomb_sprite + i].is_enabled() = false;
			}

			//load fortress
			GLuint castle = resource_dict::get_texture_handle(GL_RGBA, textures_it[7]);
			sprites[castle_sprite].init(castle, -2.50f, 0, 2, 2);
			sprites[castle_sprite].is_enabled() = false;

			// sounds
			whoosh = resource_dict::get_sound_handle(AL_FORMAT_MONO16, sounds_it[0]);
			bang = resource_dict::get_sound_handle(AL_FORMAT_MONO16, sounds_it[1]);
			death = resource_dict::get_sound_handle(AL_FORMAT_MONO16, sounds_it[2]);
			cur_source = 0;
			alGenSources(num_sound_sources, sources);

			// sundry counters and game state.
			missiles_disabled = 0;
			bombs_disabled = 50;
			invader_velocity = 0.01f;
			live_enemys = num_enemys;
			num_lives = 3;
			game_over = false;
			score = 0;
		}

		// called every frame to move things
		void simulate() {
			if (game_over) {
				return;
			}

			dodge();

			move_player();

			fire_bombs();

			move_bombs();

			//fire_missiles();
			//move_missiles();

			move_enemys(invader_velocity, 0);

			sprite &border = sprites[first_border_sprite + (invader_velocity < 0 ? 2 : 3)];
			if (enemys_collide(border)) {
				invader_velocity = -invader_velocity;
				move_enemys(invader_velocity, -0.1f);
			}
		}

		// this is called to draw the world
		void draw_world(int x, int y, int w, int h) {
			simulate();

			// set a viewport - includes whole window area
			glViewport(x, y, w, h);

			// clear the background to black
			glClearColor(1, 1, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// don't allow Z buffer depth testing (closer objects are always drawn in front of far ones)
			glDisable(GL_DEPTH_TEST);

			// allow alpha blend (transparency when alpha channel is 0)
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// draw all the sprites
			for (int i = 0; i != num_sprites; ++i) {
				sprites[i].render(texture_shader_, cameraToWorld);
			}

			char score_text[32];
			sprintf(score_text, "score: %d   lives: %d\n", score, num_lives);
			draw_text(alternative_shader_, -1.75f, 2, 1.0f / 256, score_text);

			// move the listener with the camera
			vec4 &cpos = cameraToWorld.w();
			alListener3f(AL_POSITION, cpos.x(), cpos.y(), cpos.z());
		}
	};
}
