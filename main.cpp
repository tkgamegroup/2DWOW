#include <flame/foundation/foundation.h>

using namespace flame;

int main(int argc, char** args)
{
	auto w = Window::create("Window Test", Vec2u(1280, 720), WindowFrame);

	get_looper()->loop([](Capture&, float delta_time) {
	}, Capture());

	return 0;
}
