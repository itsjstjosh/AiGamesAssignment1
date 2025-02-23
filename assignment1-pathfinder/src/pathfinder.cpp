#include "vec.hpp"
#include "draw-triangle-pro.hpp"
#include "raylib-cpp.hpp"
#include "graph.hpp"
#include "graph-utils.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <AudioDevice.hpp>

std::vector<node_t> astar_pathfind(const Graph& g, node_t start, node_t goal)
{
  std::unordered_map<node_t, node_t> came_from;
  std::unordered_map<node_t, double> cost_so_far;
  a_star_search(g, start, goal, came_from, cost_so_far);
  std::vector<node_t> path = reconstruct_path(start, goal, came_from);
  return path;
}

unsigned int path_cost(const std::vector<node_t>& path)
{
  double dcost = 0;
  if (path.size() >= 2) // then we have some lines to draw
  {
    const int num_edges = path.size() - 1;
    for (int i = 0; i < num_edges; i++)
    {
      dcost = dcost + edge_info[std::pair{path[i], path[i+1]}];
    }
  }

  return static_cast<unsigned int>(dcost);
}


int main()
{

  const int w{ 2880/2 }, h{ 1620/2 }, half_w{ w/2 }, half_h{ h/2 }, gap{ w/8 };
  raylib::Window window{ w, h, "Pathfinder" };
  raylib::AudioDevice audio; // Initialize audio device
  raylib::Sound node_add_sound("../../deps/raylib-cpp/examples/audio/resources/coin.wav");

  SetTargetFPS(60);

  Graph g;
  add_node(g, 'A', { half_w - gap, half_h });
  add_node(g, 'B', { half_w, half_h });
  add_node(g, 'C', { half_w, half_h - gap });
  add_node(g, 'D', { half_w, half_h + gap });
  add_node(g, 'E', { half_w + gap, half_h + gap });
  add_node(g, 'F', { half_w + gap, half_h });
  add_node(g, 'G', { half_w + (2 * gap), half_h - gap });
  add_double_edge(g, 'A', 'B');
  add_double_edge(g, 'B', 'C');
  add_double_edge(g, 'B', 'D');
  add_double_edge(g, 'C', 'A');
  add_double_edge(g, 'D', 'E');
  add_double_edge(g, 'D', 'A');
  add_double_edge(g, 'E', 'B');
  add_double_edge(g, 'B', 'F');
  add_double_edge(g, 'C', 'F');
  add_double_edge(g, 'C', 'G');
  add_double_edge(g, 'F', 'G');

  int t{60}; // time
  std::vector<node_t> player_path{};
  node_t start = 'A';
  node_t end   = 'G';
  player_path.push_back(start);
  //player_path.back();
  int tokens{2000}, score{}, high_score{}; // try with more/less tokens?

  while (!window.ShouldClose()) // Detect window close button or ESC key
  {
    BeginDrawing();

    ClearBackground(LIGHTGRAY);

    draw_graph(g, start, end);


    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (auto opt = get_nearby_node(GetMousePosition()))
        {
            // *opt is a node_t
            player_path.push_back(*opt); // Add the node to the player's path
            PlaySound (node_add_sound);

        }
    }

    if (player_path.size() >= 2) // Copied from raylib-redblob.cpp 
    {
        for (int i = 1; i < player_path.size(); i++)
        {
            const node_t n1 = player_path[i - 1];
            const node_t n2 = player_path[i];
            const coord_t& coord_n1 = node_info[n1];
            const coord_t& coord_n2 = node_info[n2];
            DrawLineEx(coord_n1, coord_n2, line_thickness * 2, RED);
        }
    }
                                       

    DrawText(TextFormat("Score: %d", score), 10, 10, 20, RAYWHITE);
    DrawText(TextFormat("Tokens: %d", tokens), 10, 40, 20, RAYWHITE);
    DrawText(TextFormat("High Score: %d", high_score), 10, 70, 20, RAYWHITE);
    DrawText(TextFormat("Time: %d", t), 10, 100, 20, RAYWHITE);

    EndDrawing();
  }

  CloseAudioDevice();

  return 0;
}

