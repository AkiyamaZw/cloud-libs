#include "test_with_go.h"
#include <memory>
// #include "entt/entity/registry.hpp"

namespace cloud::world::ecs::vsgo
{
size_t go_count = 0;
size_t ecs_count = 0;
size_t entt_count = 0;

void ControllerSystem::on_update(Selector &select)
{
    {
        select.for_each([](Transform &tr, Movement &move) {
            move.speed += move.accelerate * 0.033f;
            tr.x += move.speed * 0.033f;
            ecs_count += 1;
        });
    }
}
void RenderSystem::on_update(Selector &select)
{
    select.for_each([](Transform &tr, Render &render) {
        // dummy calc
        tr.x *= render.model;
        tr.y *= render.view;
        tr.z *= render.project;
        ecs_count += 1;
    });
}

void AnimatorSystem::on_update(Selector &select)
{
    select.for_each([](Transform &tr, Animation &anim) {
        anim.pose1 = tr.x * tr.x;
        anim.joint1 = tr.y * tr.y;
        anim.rotation = sqrt(tr.z);
        ecs_count += 1;
    });
}

void ControlComponent::on_update()
{
    speed += accelerate * 0.033f;
    x += speed * 0.033f;
    go_count += 1;
}

RenderComponent::RenderComponent(GameObject *_go)
    : go(_go)
{
}

void RenderComponent::on_update()
{
    auto &tr = *go->c_comp;
    // dummy calc
    tr.x *= model;
    tr.y *= view;
    tr.z *= project;
    go_count += 1;
}

AnimatorComponent::AnimatorComponent(GameObject *obj)
    : go(obj)
{
}

void AnimatorComponent::on_update()
{
    auto &tr = *go->c_comp;
    // dummy calc
    pose1 = tr.x * tr.x;
    joint1 = tr.y * tr.y;
    rotation = sqrt(tr.z);
    go_count += 1;
}

GameObject::GameObject()
{
    c_comp = new ControlComponent();
    r_comp = new RenderComponent(this);
    a_comp = new AnimatorComponent(this);
}

GameObject::~GameObject()
{
    if (c_comp)
        delete c_comp;
    if (r_comp)
        delete r_comp;
    if (a_comp)
        delete a_comp;
}

void GameObject::update()
{
    c_comp->on_update();
    r_comp->on_update();
    a_comp->on_update();
}

size_t test_ecs(size_t entity_count, size_t epoch)
{
    Registry registry;
    auto es = registry.create_entities<Transform, Movement, Render, Animation>(
        entity_count);
    ControllerSystem ctl_system;
    RenderSystem render_system;
    AnimatorSystem ani_system;
    size_t tt;
    {
        Selector ctl_select(&registry);
        ctl_select.with<Transform, Movement>();
        Selector render_select(&registry);
        render_select.with<Transform, Render>();
        Selector ani_select(&registry);
        ani_select.with<Transform, Animation>();
        Timer t;
        for (size_t i = 0; i < epoch; ++i)
        {
            ctl_system.on_update(ctl_select);
            render_system.on_update(render_select);
            ani_system.on_update(ani_select);
        }
        tt = t.get_us();
    }
    registry.destroy_entities(es);
    return tt;
}

size_t test_ecs_with_query(size_t entity_count, size_t epoch)
{
    Registry registry;
    auto es = registry.create_entities<Transform, Movement, Render, Animation>(
        entity_count);
    ControllerSystem ctl_system;
    RenderSystem render_system;
    AnimatorSystem ani_system;
    size_t tt;
    {
        // Selector ctl_select(&registry);
        // ctl_select.with<Transform, Movement>();
        // Selector render_select(&registry);
        // render_select.with<Transform, Render>();
        // Selector ani_select(&registry);
        // ani_select.with<Transform, Animation>();
        View view_ctl = Selector::query<Transform, Movement>(registry);
        auto ctl = [](Transform &tr, Movement &mo) {
            mo.speed += mo.accelerate * 0.033f;
            tr.x += mo.speed * 0.033f;
            entt_count += 1;
        };
        auto render = [](Transform &tr, Render &r) {
            tr.x *= r.model;
            tr.y *= r.view;
            tr.z *= r.project;
            entt_count += 1;
        };
        auto ani = [](Transform &tr, Animation &ani) {
            ani.pose1 = tr.x * tr.x;
            ani.joint1 = tr.y * tr.y;
            ani.rotation = sqrt(tr.z);
            entt_count += 1;
        };
        Timer t;
        for (size_t i = 0; i < epoch; ++i)
        {
            // ctl_system.on_update(ctl_select);
            // render_system.on_update(render_select);
            // ani_system.on_update(ani_select);
            view_ctl.each(ctl);
        }
        tt = t.get_us();
    }
    registry.destroy_entities(es);
    return tt;
}

size_t test_go(size_t entity_count, size_t epoch)
{
    std::vector<std::shared_ptr<GameObject>> gos;
    for (size_t i = 0; i < entity_count; ++i)
    {
        gos.emplace_back(std::make_shared<GameObject>());
    }

    {
        Timer t;
        for (size_t e = 0; e < epoch; ++e)
        {
            for (size_t i = 0; i < gos.size(); ++i)
                gos[i]->update();
        }
        return t.get_us();
    }
}

void test_main()
{
    size_t entity_cnt = 100000;
    size_t epochs = 10;
    int average_cnt = 10;
    using TestFunc = std::function<size_t(size_t, size_t)>;

    std::cout << "entity count: " << entity_cnt << " update times: " << epochs
              << " times: " << average_cnt << std::endl;

    auto show_result = [](const std::string &name,
                          const std::pair<float, size_t> &data) {
        std::cout << std::format(
                         "tag: [{:>12}], avg: {:>8} us, total: {:>8} us",
                         name,
                         data.first,
                         data.second)
                  << std::endl;
    };

    auto calc_average = [average_cnt, entity_cnt, epochs](
                            TestFunc func) -> std::pair<float, size_t> {
        size_t acculate_time = 0;
        for (int i = 0; i < average_cnt; ++i)
            acculate_time += func(entity_cnt, epochs);
        return {1.0f * acculate_time / average_cnt, acculate_time};
    };

    show_result("gameobjct", calc_average(test_go));
    show_result("myecs", calc_average(test_ecs));
    // show_result("myecs_query", calc_average(test_ecs_with_query));
    // assert(ecs_count == go_count && go_count == entt_count && ecs_count ==
    // entt_count);
}

} // namespace cloud::world::ecs::vsgo