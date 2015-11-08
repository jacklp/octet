#Medieval Pong




##Youtube Link
https://www.youtube.com/watch?v=FqZitkiHglE



##Controls

W - Moves player up along the Y-axis.
S - Moves player down along the Y-axis.



##Game Objective

- You are a shield.
- Defend your castle by reflecting arrows back on to the attackers.
- There are 20 enemies.



##Shaders

texture_shader.h:
- The fragment shader takes the output of the texture2D function (vec4) and reverses the rgb components of the pixel to invert the colour.

alternative_shader.h:
- The fragment shader sets the pixel colour to red only and set's the green and blue to 0. 
- Similarly to the texture shader it ignores the alpha part.

checker_shader.h:
- The fragment shader gives us the relative position of gl_FragCoord to a 100 * 100 square. It then works out which quadrant
it is in and returns either black or white to give us a checkerboard pattern.



##Overloading

![Overloading diagram](http://i.imgur.com/aE5JLSV.jpg)

sprite.h has 2 render methods:
>void render(texture_shader &shader, mat4t &cameraToWorld)
>void render(checker_shader &shader, mat4t &cameraToWorld)

- As C++ is a static typed language; at compile type the program associates which version of render to associate with the types of input.
- Later on at runtime we check a boolean on the sprite to establish which version of the render function should be run. 
- Method overloading should not be confused with forms of polymorphism where the correct method is chosen at runtime e.g through virtual functions.



##tinyxml - Third Party Library

I implemented a utilities library to do 2 things:
- invaderers_app::load_xml tinyxml() reads the document file in to memory.
- invaderers_app::load_assets_via_xml() has 2 references to vectors passed in as parameters. It parses the element nodes and stores them in the relevant vector.



##Iterators

- The sound and texture vectors in invaderers_app::app_init() are assigned iterators after they have been filled with data from config.xml.
- The iterators are later used to point to the relevant index to retrieve the string of the file paths for the assets of the application.



##Reworked key mappings

- in windows_specific::translate() I replaced the existing Windows arrow key references (e.g VK_LEFT) with the ASCII values of WASD (e.g 65)



##Gameplay

- Changed the player controls to move along the vertical axis.
- Flipped the game effectively horizontally.
- Changed the scoring to work based off of bullet collision with the castle sprite.
- The enemy sprite's are instantiated off screen to the right at a random height using the randomizer class.
- Enemy sprite pathing only moves left until they get to a x-position relative to their index and a fixed variable. 
- The bullets reflect back in the opposite direction along the x axis when they hit the player.



##Assets - Sounds and Images

- The shield gif was created in adobe illustrator, the rest were stock images found online and modified in photoshop.
- The sound wav was found online and implemented.



##Sequential Full Patch Notes

1. Load in a config via XML using the TinyXML parser library.
2. Create an iterator for our vector<string> textures and use this to plug in the correct attributes to application (this could be made more dynamic).
3. Modify the input keys from arrow keys to wasd.
4. Add an additional move for the spaceship - hit space to jump in your currently moving direction.
5. "invert_color" function to modify the texture_shader to invert the colours of all the textures.
6. change the background colour to white.
7. Added tinkle.wav sound to be played upon death.
8. Created new shader - alternative_shader.
9. Draw_text now uses custom shader with "make_red" function.
10. added new graphics: castle.gif, enemy.gif, pong_bar.gif
11. redrew the borders to make the game horizontal and smaller.
12. reconfigured start position for player and castle to the left side of the screen.
13. reconfigured the enemys to start offscreen to the right and in randomly generated y positions in pairs.
14. changed the is_above method on sprite to is_adjacent and now checks for horizontal position in the modeltoworld vector
15. the bullets now move horizontally and check for collision against the left wall.
16. key_down was added to windows_specific and the key bindings changed to move the player up and down.
17. rotated bomb.gif
18. changed the scoring system to deduct lives if the bombs hit the castle.
19. removed missile functionality.
20. added bounce back functionality to the player so that the bombs travel in the opposite direction if they collide with the player.
21. added collision detection functionality so that if the bombs bounce back and hit an enemy it kills the enemy.
22. completely changed fire bombs functionality so that bombs are only fired by on_screen enemys that are stored in a temporary vector
23. player movement speed increased. Removed the dodge functionality as it is now defunct.
24. enemy pathing solved.
25. changed player graphic to a medieval shield
26. Overloaded sprite::render() function to pass a "checker_shader" if sprite::is_border == true;
27. modified checker_shader's fragment shader to create a checker pattern.