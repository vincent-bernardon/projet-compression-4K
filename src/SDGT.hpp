#ifndef SDGT_HPP
#define SDGT_HPP

#include <opencv2/opencv.hpp>
#include "Superpixel.hpp"
#include <queue>
#include <tuple>

struct GraphNode{
    Superpixel * superpixel;
    std::unordered_map<GraphNode*, float> neighbors;
    bool ignore = false;
    int index;

    GraphNode(Superpixel * _superpixel, int _index){
        superpixel = _superpixel;
        index = _index;
    }

    void mergeColor(GraphNode * other){
        int totalPixels = superpixel->nbPixels + other->superpixel->nbPixels;
        superpixel->lab[0] = (superpixel->lab[0] * superpixel->nbPixels + other->superpixel->lab[0] * other->superpixel->nbPixels) / totalPixels;
        superpixel->lab[1] = (superpixel->lab[1]* superpixel->nbPixels + other->superpixel->lab[1] * other->superpixel->nbPixels) / totalPixels;
        superpixel->lab[2] = (superpixel->lab[2] * superpixel->nbPixels + other->superpixel->lab[2] * other->superpixel->nbPixels) / totalPixels;
    
        superpixel->nbPixels = totalPixels;
    }

};


struct Graph {

    std::vector<GraphNode> nodes;
    // priority queue pour les distances des liens
    std::priority_queue<std::tuple<float, GraphNode*, GraphNode*>, 
                        std::vector<std::tuple<float, GraphNode*, GraphNode*>>, 
                        std::greater<std::tuple<float, GraphNode*, GraphNode*>>> edgeQueue;

    
    std::vector<std::vector<int>> superpixelToPixels;
    
    Graph(std::vector<Superpixel> & superpixel, std::vector<int>& pixelToSuperpixel, int nbSpLines, int nbSpCols){
        for(int i = 0; i < superpixel.size(); i++){
            nodes.push_back(GraphNode(&superpixel[i], i));
        }

        for (size_t i = 0 ; i < nbSpLines ; i++){
            for (size_t j = 0 ; j < nbSpCols ; j++){
                GraphNode& current = nodes[  (i * nbSpCols) + j ];
                if (i != nbSpLines -1){ 
                    GraphNode& bottom_Neighbor = nodes[  ((i + 1) * nbSpCols) + j ];
                    float dist = getDistanceLAB(current.superpixel->lab, bottom_Neighbor.superpixel->lab, 1.0f, 1.0f, 1.0f);
                    current.neighbors[&bottom_Neighbor] = dist;
                    bottom_Neighbor.neighbors[&current] = dist;
                }
                if (j != nbSpCols -1){
                    GraphNode& right_Neighbor = nodes[  (i * nbSpCols) + j+1 ];
                    float dist = getDistanceLAB(current.superpixel->lab, right_Neighbor.superpixel->lab, 1.0f, 1.0f, 1.0f);
                    current.neighbors[&right_Neighbor] = dist;
                    right_Neighbor.neighbors[&current] = dist;
                }
            }      
        }

        for (auto& node : nodes) {
            if (node.ignore) continue;
            for (auto& neighbor : node.neighbors) {
                if (node.index < neighbor.first->index) { 
                    edgeQueue.push(std::make_tuple(neighbor.second, &node, neighbor.first));
                }
            }
        }

        initSuperPixeltoPixelVector(pixelToSuperpixel);

    };

    void mergeClosestSuperpixel(std::vector<int>& pixelToSuperpixel){
        //  recup la plus petite distance de la priority queue
        while (!edgeQueue.empty()) {
            auto edge = edgeQueue.top();
            edgeQueue.pop();
            
            float distance = std::get<0>(edge);
            GraphNode* node1 = std::get<1>(edge);
            GraphNode* node2 = std::get<2>(edge);
            
            // si c'est un noeud deja "fusionné" on skip
            if (node1->ignore || node2->ignore) continue;

            // ou skip si les noeuds sont plus voisins
            if (node1->neighbors.find(node2) == node1->neighbors.end()) continue;
            
            // on a la plus petite distance entre 2 noeuds 
            GraphNode* GraphNodeToRedirectTo = node1;
            GraphNode* GraphNodeToDelete = node2;
            
            GraphNodeToRedirectTo->mergeColor(GraphNodeToDelete);
        
            // parcourir tous les noeuds, 
            for (auto & node : nodes){
                if (node.ignore){ // noeuds qui ont été "supprimé"
                    continue;
                }
                auto it = node.neighbors.find(GraphNodeToDelete);
                // si voisin avec to delete
                if (it != node.neighbors.end()) {
                    //std::cout << "suppression d'un voisinage" << std::endl;
                    node.neighbors.erase(it);
                    // supprimmer le voisinage avec le noeud a delete et on en ajoute un noveau vers GraphNodeToRedirectTo et on la distance 
                    if (&node != GraphNodeToRedirectTo) {

                        float newDist = getDistanceLAB(node.superpixel->lab, GraphNodeToRedirectTo->superpixel->lab, 1.0f, 1.0f, 1.0f);
                        node.neighbors[GraphNodeToRedirectTo] = newDist;
                
                        GraphNodeToRedirectTo->neighbors[&node] = newDist;
                    }
                }
            }

            GraphNodeToDelete->ignore = true;
            GraphNodeToDelete->neighbors.clear();


            // on met a jour les pixels associé au superpixel a supprimer pour qu'ils soit associé a celui qui va le remplacer
            for (int pixelIdx : superpixelToPixels[GraphNodeToDelete->index]) {
                pixelToSuperpixel[pixelIdx] = GraphNodeToRedirectTo->index;
            }

            // déplace les ids des pixels du node a supprimer vers celui qui va le remplacer
            superpixelToPixels[GraphNodeToRedirectTo->index].insert(
                superpixelToPixels[GraphNodeToRedirectTo->index].end(),
                superpixelToPixels[GraphNodeToDelete->index].begin(),
                superpixelToPixels[GraphNodeToDelete->index].end()
            );
            superpixelToPixels[GraphNodeToDelete->index].clear(); 

            // on ajoute à la queue les nouvelles connexions.
            for (auto& neighbor : GraphNodeToRedirectTo->neighbors) {
                if (GraphNodeToRedirectTo->index < neighbor.first->index) {
                    edgeQueue.push(std::make_tuple(neighbor.second, GraphNodeToRedirectTo, neighbor.first));
                }
            }

            break;
        }
    }

    // vecteur avec indice = id superpixel, valeur = pixel
    void initSuperPixeltoPixelVector(std::vector<int>& pixelToSuperpixel){
        superpixelToPixels.clear();
        superpixelToPixels.resize(nodes.size());
        for (size_t i = 0; i < pixelToSuperpixel.size(); i++) {
            int spIndex = pixelToSuperpixel[i];
            if (spIndex >= 0 && spIndex < superpixelToPixels.size()) {
                superpixelToPixels[spIndex].push_back(i);
            }else {
                std::cout << "erreur, indice de pixel vers superpixel , " << spIndex << std::endl;
            }
        }
    }

    void printGraphInfo() {
        std::cout << "=== Graph Debug Information ===" << std::endl;
        std::cout << "Total nodes: " << nodes.size() << std::endl;
        
        for (size_t i = 0; i < nodes.size(); i++) {
            GraphNode& node = nodes[i];
            if (node.ignore) continue;

            
            std::cout << "\nNode " << i << " at position (" 
                      << node.superpixel->x << ", " 
                      << node.superpixel->y << "):" << std::endl;
            std::cout << "  Lab color: [" 
                      << node.superpixel->lab[0] << ", " 
                      << node.superpixel->lab[1] << ", " 
                      << node.superpixel->lab[2] << "]" << std::endl;
            std::cout << "  Number of neighbors: " << node.neighbors.size() << std::endl;
            std::cout << "  Is ignored: " << node.ignore << std::endl;
            if(!node.ignore) std::cout << "  Number of associated pixels " << node.superpixel->nbPixels << std::endl;

            
            if (node.neighbors.size() > 0) {
                std::cout << "  Neighbors:" << std::endl;
                int neighborCount = 0;
                for (const auto& neighbor_pair : node.neighbors) {
                    GraphNode* neighbor = neighbor_pair.first;
                    float distance = neighbor_pair.second;
                    
                    // Find neighbor's index
                    size_t neighborIndex = 0;
                    for (size_t j = 0; j < nodes.size(); j++) {
                        if (&nodes[j] == neighbor) {
                            neighborIndex = j;
                            break;
                        }
                    }
                    
                    std::cout << "    -> Node " << neighborIndex 
                              << " at (" << neighbor->superpixel->x << ", " 
                              << neighbor->superpixel->y << ")"
                              << " with distance " << distance << std::endl;
                    neighborCount++;
                }
            }
        }
        std::cout << "=============================" << std::endl;
    }

};


 
cv::Mat SDGT(char* imagePath, int k, int m, int mp, float coeff);

#endif