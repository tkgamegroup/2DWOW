#include <flame/utils/app.h>
#include <flame/universe/components/image.h>

using namespace flame;

App g_app;

Window* window;
Entity* root;

graphics::ImageAtlas* atlas_character;
uint atlas_character_id;

struct Map
{
	Entity* e;
	cElement* ce;

	void create()
	{
		e = Entity::create();
		ce = cElement::create();
		ce->set_width(10000.f);
		ce->set_height(10000.f);
		ce->set_border(1.f);
		ce->set_border_color(Vec4c(255));
		e->add_component(ce);
		root->add_child(e);
	}

	void update_view();
}map;

struct Sprite
{
	Entity* e;
	cElement* ce;

	Vec2f pos = Vec2f(0.f);
	float rotation = 0.f;

	void create(const Vec2f& p)
	{
		e = Entity::create();
		ce = cElement::create();
		ce->set_x(pos.x());
		ce->set_y(pos.y());
		ce->set_width(64.f);
		ce->set_height(64.f);
		ce->set_fill_color(Vec4c(255, 0, 0, 255));
		e->add_component(ce);
		map.e->add_child(e);

		set_pos(p);
	}

	void set_pos(const Vec2f& p)
	{
		pos = p;
		ce->set_x(pos.x());
		ce->set_y(pos.y());
	}

	void add_pos(const Vec2f& p)
	{
		pos += p;
		ce->set_x(pos.x());
		ce->set_y(pos.y());
	}

	void set_rotation(float r)
	{
		rotation = r;
		ce->set_rotation(rotation);
	}

	void add_rotation(float r)
	{
		rotation += r;
		ce->set_rotation(rotation);
	}
};

struct Animation : Sprite
{
	cImage* ci;

	int frame = -1;
	std::vector<uint> frames;

	void create(Entity* parent)
	{
		e = Entity::create();
		ce = cElement::create();
		ce->set_width(64.f);
		ce->set_height(64.f);
		ce->set_pivotx(0.5f);
		ce->set_pivoty(0.5f);
		e->add_component(ce);
		ci = cImage::create();
		ci->set_auto_size(false);
		e->add_component(ci);
		parent->add_child(e);

		set_pos(Vec2f(32.f, 0.f));
	}

	void set_frames(graphics::ImageAtlas* atlas, uint atlas_id, const std::vector<std::string>& names)
	{
		ci->set_res_id(atlas_id);

		frames.resize(names.size());
		for (auto i = 0; i < names.size(); i++)
		{
			auto tile = atlas->find_tile(names[i].c_str());
			assert(tile);
			frames[i] = tile->get_index();
		}
	}

	void play()
	{
		if (frame == -1)
			frame = 0;
	}

	void advance()
	{
		if (frame != -1)
		{
			if (frame == frames.size())
			{
				frame = -1;
				ci->set_tile_id(-1);
			}
			else
				ci->set_tile_id(frames[frame++]);
		}
	}
};

struct Character : Sprite
{

};

struct Player : Sprite
{
	bool w = false;
	bool s = false;
	bool a = false;
	bool d = false;

	Animation ani_smash;

	void create()
	{
		e = Entity::create();
		ce = cElement::create();
		ce->set_width(64.f);
		ce->set_height(64.f);
		ce->set_pivotx(0.5f);
		ce->set_pivoty(0.5f);
		e->add_component(ce);
		auto ci = cImage::create();
		ci->set_src("character.main");
		ci->set_auto_size(false);
		e->add_component(ci);
		map.e->add_child(e);

		set_pos(Vec2f(100.f, 200.f));

		ani_smash.create(e);
		ani_smash.set_frames(atlas_character, atlas_character_id, { "smash1", "smash2", "smash3" });
	}

	void update()
	{
		auto dir = Vec2f(0.f);
		if (w && !s)
		{
			auto rad = (rotation - 90.f) * ANG_RAD;
			dir += Vec2f(cos(rad), sin(rad));
		}
		if (s && !w)
		{
			auto rad = (rotation - 90.f) * ANG_RAD;
			dir -= Vec2f(cos(rad), sin(rad));
		}
		if (a && !d)
		{
			auto rad = rotation * ANG_RAD;
			dir -= Vec2f(cos(rad), sin(rad));
		}
		if (d && !a)
		{
			auto rad = rotation * ANG_RAD;
			dir += Vec2f(cos(rad), sin(rad));
		}
		if (dir != 0.f)
			add_pos(normalize(dir) * 3.f);

		ani_smash.advance();
	}
}player;

void Map::update_view()
{
	auto wnd_size = Vec2f(window->get_size());
	auto v = rotation(-player.rotation * ANG_RAD) * -player.pos;
	ce->set_x(wnd_size.x() * 0.5f + v.x());
	ce->set_y(wnd_size.y() * 0.75f + v.y());
	ce->set_rotation(-player.rotation);
}

struct MainWindow : GraphicsWindow
{
	MainWindow() :
		GraphicsWindow(&g_app, true, true, "", Vec2u(1280, 720), WindowFrame)
	{
		window->set_cursor(CursorNone);

		s_element_renderer->set_always_update(true);

		atlas_character = graphics::ImageAtlas::create(g_app.graphics_device, L"../art/character.atlas");
		atlas_character_id = canvas->set_resource(-1, atlas_character, "character");
	}

	void on_frame() override
	{
		player.update();

		map.update_view();
	}
};

int main(int argc, char** args)
{
	g_app.create();
	new MainWindow();

	window = g_app.main_window->window;
	root = g_app.main_window->root;

	map.create();

	Sprite s1, s2;
	s1.create(Vec2f(100.f));
	s2.create(Vec2f(200.f));
	player.create();

	map.update_view();

	auto cer = (cEventReceiver*)root->get_component(cEventReceiver::type_hash);
	cer->add_key_down_listener([](Capture&, KeyboardKey key) {
		switch (key)
		{
		case Keyboard_W:
			player.w = true;
			break;
		case Keyboard_S:
			player.s = true;
			break;
		case Keyboard_A:
			player.a = true;
			break;
		case Keyboard_D:
			player.d = true;
			break;
		}
	}, Capture());
	cer->add_key_up_listener([](Capture&, KeyboardKey key) {
		switch (key)
		{
		case Keyboard_W:
			player.w = false;
			break;
		case Keyboard_S:
			player.s = false;
			break;
		case Keyboard_A:
			player.a = false;
			break;
		case Keyboard_D:
			player.d = false;
			break;
		}
	}, Capture());
	cer->add_mouse_move_listener([](Capture&, const Vec2i& disp, const Vec2i& pos) {
		static int px = -1;
		if (px != -1)
			player.add_rotation((pos.x() - px) * 0.3f);
		auto p = window->get_pos() + Vec2f(window->get_size()) * 0.5f;
		set_mouse_pos(p);
		px = window->global_to_local(p).x();
	}, Capture());
	cer->add_mouse_left_down_listener([](Capture&, const Vec2i& pos) {
		player.ani_smash.play();
	}, Capture());

	g_app.run();

	return 0;
}
