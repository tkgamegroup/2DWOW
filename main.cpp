#include <flame/utils/app.h>

using namespace flame;

App g_app;

Entity* root;

Entity* map;
Entity* player;

int main(int argc, char** args)
{
	g_app.create();
	new GraphicsWindow(&g_app, true, true, "", Vec2u(1280, 720), WindowFrame);

	root = g_app.main_window->root;
	map = Entity::create();
	{
		auto ce = cElement::create();
		map->add_component(ce);
	}
	root->add_child(map);
	{
		auto o = Entity::create();
		auto ce = cElement::create();
		ce->set_width(50.f);
		ce->set_height(50.f);
		ce->set_fill_color(Vec4c(255, 0, 0, 255));
		o->add_component(ce);
		map->add_child(o);
	}
	{
		auto o = Entity::create();
		auto ce = cElement::create();
		ce->set_x(500.f);
		ce->set_width(50.f);
		ce->set_height(50.f);
		ce->set_fill_color(Vec4c(255, 0, 0, 255));
		o->add_component(ce);
		map->add_child(o);
	}
	{
		auto o = Entity::create();
		auto ce = cElement::create();
		ce->set_x(100.f);
		ce->set_y(200.f);
		ce->set_width(50.f);
		ce->set_height(50.f);
		ce->set_pivotx(0.5f);
		ce->set_pivoty(0.5f);
		ce->set_rotation(10.f);
		ce->set_fill_color(Vec4c(255, 255, 255, 255));
		o->add_component(ce);
		map->add_child(o);
		player = o;
	}

	{
		auto ce = (cElement*)map->get_component(cElement::type_hash);
		auto wnd_size = Vec2f(g_app.main_window->window->get_size());
		auto v = rotation(-10.f * ANG_RAD) * (-Vec2f(100.f, 200.f));
		ce->set_x(wnd_size.x() * 0.5f + v.x());
		ce->set_y(wnd_size.y() * 0.5f + v.y());
		ce->set_rotation(-10.f);
	}

	g_app.run();

	return 0;
}
