resources
{
	path "../../common" filter "(\\w)+.rs" //Windows/Linux
	path "common" filter "(\\w)+.rs" //macOS
	path "./**" filter "(\\w)+.(ac|acgz|acj|acjgz|sm3dgz)"
}