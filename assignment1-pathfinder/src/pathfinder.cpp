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

//for time delay
#include <iostream>
#include <chrono>
#include <thread>
using namespace std;


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
  raylib::AudioDevice audio; // Initialize audio device - k
  raylib::Sound node_add_sound("../../deps/raylib-cpp/examples/audio/resources/coin.wav"); // Add audio from path - k

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
  int tokens{2000}, score{}, high_score{}; // try with more/less tokens?
  int tokenCost= 0;

  bool gameNeedsReset = false;
  double startTime = GetTime();

  while (!window.ShouldClose()) // Detect window close button or ESC key
  {
    BeginDrawing();

    ClearBackground(LIGHTGRAY);

    draw_graph(g, start, end);

    //astar_pathfind(g, start, end);

    if (GetTime() - startTime >= 1.0f) // Timer countdown each second using GetTime() from raylib-cpp - k
    {
        if (t > 0) // Prevent negative time - k
        {
            t--; // Decrease t (time) by 1 
        }
        startTime = GetTime(); 
    }
    

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        
        if (auto opt = get_nearby_node(GetMousePosition()))
        {

            node_t current = player_path.back();
            node_t next = *opt;

            //the code before the && stops the player from clicking previously clicked nodes, idk if we should keep that or not
            if (std::find(player_path.begin(), player_path.end(), next) == player_path.end() && edge_info.find(std::make_pair(current, next)) != edge_info.end()) //got this working after 3 hours of google and frankensteining code -J
            {
                player_path.push_back(next); // Add the node to the player's path
                PlaySound (node_add_sound);
                

                tokenCost =  edge_info[std::make_pair(current, next)];
                tokens = tokens - tokenCost; //remove tokens if the next node is succesfully clicked -j
                

            }
            
            if (current != start)  
            {
                if (current == next)
                {
                    if (player_path.size() >= 2) 
                    {
                        node_t previous = player_path[player_path.size() - 2];
                        
                        player_path.pop_back();
                    
                        tokenCost =  edge_info[std::make_pair(next, previous)];
                        tokens = tokens + tokenCost; //add tokens if the node is unselected -j
                    }
                    
                    

                }
            }
            
            if (next == end) 
            {
                //calculate the ideal cost - j
                std:: vector<node_t> idealPath = astar_pathfind(g, start, end);
                int idealCost = path_cost(idealPath);
            
                //calculate the players path cost - j
                int playersCost = path_cost(player_path);

                if (playersCost == idealCost) 
                {
                    score = score + idealCost;
                }
                else if (playersCost > idealCost) 
                { 
                    //gets the ammount spent over the ideal cost so it can be subtracted - j
                    int PlayersOverSpending = playersCost - idealCost;

                    //subtracts the ammount over the ideal cost from the ideal cost so the player is punished the more they go over the ideal cost. - j
                    score = score + idealCost - PlayersOverSpending;   
                    
                    if (score < 0) 
                    {
                        score = 0; //stops the player getting negative scores
                    }
                }
                player_path.clear();
                player_path.push_back(start); // Add clicked node to player path - k
                tokens = 2000;
                t = 60; // Reset timer to 60 - k
            }
            

            // *opt is a node_t

        }
    }

    if (player_path.size() >= 2) // Copied from raylib-redblob.cpp - k
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
                                       
    // Adds text of score, tokens, high score and time to the window in white colour - k
    DrawText(TextFormat("Score: %d", score), 10, 10, 20, RAYWHITE);
    DrawText(TextFormat("Tokens: %d", tokens), 10, 40, 20, RAYWHITE);
    DrawText(TextFormat("High Score: %d", high_score), 10, 70, 20, RAYWHITE);
    DrawText(TextFormat("Time: %d", t), 10, 100, 20, RAYWHITE);

    EndDrawing();
  }

  CloseAudioDevice();

  return 0;
}

