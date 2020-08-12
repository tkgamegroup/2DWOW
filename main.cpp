#include <flame/utils/app.h>

using namespace flame;

App g_app;

Entity* root;

Entity* map;
cElement* map_element;
Entity* player;
cElement* player_element;
Vec2f player_pos = Vec2f(100.f, 200.f);
float player_dir = 120.f;
bool player_w = false;

void update_view()
{
	player_element->set_rotation(player_dir);

	auto wnd_size = Vec2f(g_app.main_window->window->get_size());
	auto v = rotation(-player_dir * ANG_RAD) * (-Vec2f(player_pos.x(), player_pos.y()));
	map_element->set_x(wnd_size.x() * 0.5f + v.x());
	map_element->set_y(wnd_size.y() * 0.75f + v.y());
	map_element->set_rotation(-player_dir);
}

struct MainWindow : GraphicsWindow
{
	MainWindow() :
		GraphicsWindow(&g_app, true, true, "", Vec2u(1280, 720), WindowFrame)
	{
	}

	void on_frame() override
	{
		if (player_w)
		{
			auto rad = (player_dir - 90.f) * ANG_RAD;
			player_pos += Vec2f(cos(rad), sin(rad)) * 3.f;
			player_element->set_x(player_pos.x());
			player_element->set_y(player_pos.y());
		}

		update_view();

		canvas->add_text(0, std::to_wstring(player_dir).c_str(), nullptr, 14, Vec4c(255), Vec2f(0.f));
	}
};

int main(int argc, char** args)
{
	g_app.create();
	new MainWindow();

	g_app.main_window->s_element_renderer->set_always_update(true);

	root = g_app.main_window->root;
	map = Entity::create();
	{
		auto ce = cElement::create();
		ce->set_width(10000.f);
		ce->set_height(10000.f);
		ce->set_border(1.f);
		ce->set_border_color(Vec4c(255));
		map->add_component(ce);
		map_element = ce;
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
		ce->set_x(player_pos.x());
		ce->set_y(player_pos.y());
		ce->set_width(50.f);
		ce->set_height(50.f);
		ce->set_pivotx(0.5f);
		ce->set_pivoty(0.5f);
		ce->set_fill_color(Vec4c(255, 255, 255, 255));
		o->add_component(ce);
		map->add_child(o);
		player = o;
		player_element = ce;
	}

	update_view();

	g_app.main_window->window->set_cursor(CursorNone);

	auto cer = (cEventReceiver*)root->get_component(cEventReceiver::type_hash);
	cer->add_key_down_listener([](Capture&, KeyboardKey key) {
		if (key == Keyboard_W)
			player_w = true;
	}, Capture());
	cer->add_key_up_listener([](Capture&, KeyboardKey key) {
		if (key == Keyboard_W)
			player_w = false;
	}, Capture());
	cer->add_mouse_move_listener([](Capture&, const Vec2i& disp, const Vec2i& pos) {
		static int px = -1;
		if (px != -1)
			player_dir += (pos.x() - px) * 0.3f;
		auto w = g_app.main_window->window;
		auto p = w->get_pos() + Vec2f(w->get_size()) * 0.5f;
		set_mouse_pos(p);
		px = w->global_to_local(p).x();
	}, Capture());

	g_app.run();

	return 0;
}
