resources
{
	path "../../common" filter "(\\w)+.rs" //Windows/Linux
	path "common" filter "(\\w)+.rs" //macOS
	//recursive: a file is named by its path relative to assets ("base/scene", ...)
	path "assets/**"
}