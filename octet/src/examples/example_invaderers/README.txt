patch notes:

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