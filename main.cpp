#include <flame/utils/app.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/image.h>
#include <flame/universe/components/tile_map.h>

using namespace flame;

App g_app;

Window* window;
Entity* root;

graphics::ImageAtlas* atlas_main;
uint atlas_main_id;
int grass_id;
int stone_id;

cText* tip;

struct Map
{
	inline static const auto cx = 100;
	inline static const auto cy = 100;

	int cells[cy][cx];

	Entity* e;
	cElement* ce;
	cTileMap* ct;

	void create()
	{
		for (auto i = 0; i < cy; i++)
		{
			for (auto j = 0; j < cx; j++)
				cells[i][j] = -1;
		}

		e = Entity::create();
		ce = cElement::create();
		ce->set_width(64.f * cx);
		ce->set_height(64.f * cy);
		ce->set_border(1.f);
		ce->set_border_color(Vec4c(255));
		e->add_component(ce);
		ct = cTileMap::create();
		ct->set_res_id(atlas_main_id);
		ct->set_cell_count(Vec2u(cx, cy));
		ct->set_cell_size(Vec2f(64.f));
		for (auto i = 0; i < cy; i++)
		{
			for (auto j = 0; j < cx; j++)
				set_cell(Vec2u(j, i), (rand() % 100) >= 5 ? grass_id : stone_id);
		}
		ct->set_clipping(true);
		e->add_component(ct);
		root->add_child(e);
	}

	int get_cell(const Vec2u& idx)
	{
		return cells[idx.y()][idx.x()];
	}

	void set_cell(const Vec2u& idx, int t)
	{
		cells[idx.y()][idx.x()] = t;

		ct->set_cell(idx, t, Vec4c(255));
	}

	void update_view();
}map;

struct Sprite
{
	Entity* e;
	cElement* ce;
	cImage* ci;

	Vec2f pos = Vec2f(0.f);
	float rotation = 0.f;

	void create(Entity* parent, const Vec2f& p, const Vec2f& pivot = Vec2f(0.f))
	{
		e = Entity::create();
		ce = cElement::create();
		ce->set_width(64.f);
		ce->set_height(64.f);
		ce->set_pivotx(pivot.x());
		ce->set_pivoty(pivot.y());
		e->add_component(ce);
		ci = cImage::create();
		ci->set_auto_size(false);
		e->add_component(ci);
		parent->add_child(e);

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
	int frame = -1;
	std::vector<uint> frames;

	void create(Entity* parent, const Vec2f& p)
	{
		Sprite::create(parent, p, Vec2f(0.5f));
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
		{
			frame = 0;
			looper().add_event([](Capture& c) {
				auto thiz = c.thiz<Animation>();
				if (thiz->frame == thiz->frames.size())
				{
					thiz->frame = -1;
					thiz->ci->set_tile_id(-1);
				}
				else
				{
					thiz->ci->set_tile_id(thiz->frames[thiz->frame]);
					thiz->frame++;
					c._current = INVALID_POINTER;
				}
			}, Capture().set_thiz(this));
		}
	}
};

struct Monster : Sprite
{
	void create(const Vec2f& p)
	{
		Sprite::create(map.e, p, Vec2f(0.5f));
		ci->set_src("main.slime");
	}
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
		Sprite::create(map.e, Vec2f(64.f) * 50.f, Vec2f(0.5f));
		ci->set_src("main.character");

		ani_smash.create(e, Vec2f(32.f, 0.f));
		ani_smash.set_frames(atlas_main, atlas_main_id, { "smash1", "smash2", "smash3" });
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
		{
			dir = normalize(dir) * 3.f;
			auto idx = Vec2u(pos / 64.f);
			std::vector<Vec2u> occluders;
			if (idx.x() > 0)
				occluders.push_back(Vec2u(idx.x() - 1, idx.y()));
			if (idx.x() < Map::cx - 1)
				occluders.push_back(Vec2u(idx.x() + 1, idx.y()));
			if (idx.y() > 0)
				occluders.push_back(Vec2u(idx.x(), idx.y() - 1));
			if (idx.y() < Map::cy - 1)
				occluders.push_back(Vec2u(idx.x(), idx.y() + 1));
			if (idx.x() > 0 && idx.y() > 0)
				occluders.push_back(Vec2u(idx.x() - 1, idx.y() - 1));
			if (idx.x() > 0 && idx.y() < Map::cy - 1)
				occluders.push_back(Vec2u(idx.x() - 1, idx.y() + 1));
			if (idx.x() < Map::cx - 1 && idx.y() > 0)
				occluders.push_back(Vec2u(idx.x() + 1, idx.y() - 1));
			if (idx.x() < Map::cx - 1 && idx.y() < Map::cy - 1)
				occluders.push_back(Vec2u(idx.x() + 1, idx.y() + 1));
			const auto r = 30.f;
			const auto r_sq = r * r;
			auto p = pos + dir;
			for (auto& o : occluders)
			{
				if (map.get_cell(o) != grass_id)
				{
					auto LT = Vec2f(o) * 64.f;
					auto RB = Vec2f(o + 1U) * 64.f;
					auto c = clamp(p, LT, RB);
					auto d_sq = distance_square(p, c);
					if (d_sq < r_sq)
						p += normalize(p - c) * (r - sqrt(d_sq));
				}
			}
			set_pos(p);
		}
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
		GraphicsWindow(&g_app, true, true, "", Vec2u(640), WindowFrame)
	{
		window->set_cursor(CursorNone);

		s_element_renderer->set_always_update(true);

		atlas_main = graphics::ImageAtlas::create(g_app.graphics_device, L"../art/main.atlas");
		atlas_main_id = canvas->set_resource(-1, atlas_main, "main");
		grass_id = atlas_main->find_tile("grass")->get_index();
		stone_id = atlas_main->find_tile("stone")->get_index();
	}

	void on_frame() override
	{
		player.update();

		map.update_view();

		tip->set_text((to_wstring(player.pos) + L"\n" + to_wstring(Vec2u(player.pos / 64.f))).c_str());
	}
};

int main(int argc, char** args)
{
	g_app.create();
	new MainWindow();

	window = g_app.main_window->window;
	root = g_app.main_window->root;

	map.create();

	Monster m1;
	m1.create(Vec2f(64.f) * 48.f);

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

	{
		auto e = Entity::create();
		e->add_component(cElement::create());
		tip = cText::create();
		tip->set_color(Vec4c(255));
		e->add_component(tip);
		root->add_child(e);
	}

	g_app.run();

	return 0;
}
