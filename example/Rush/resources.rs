resources
{
	path "../../common" filter "(\\w)+.rs" //Windows/Linux
	path "common" filter "(\\w)+.rs" //macOS
	//the effects and the shaders of the game (the trails of the hovercraft in the grounds)
	path "effect" filter "(\\w)+.sqfx"
	path "shader" filter "(\\w)+.hlsl"
	//recursive: a file is named by its path relative to assets ("arena/scene", "hovercraft/scene")
	path "assets/**"
}