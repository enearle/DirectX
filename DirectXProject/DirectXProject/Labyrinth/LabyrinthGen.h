#pragma once
#include <ctime>
#include <DirectXMath.h>
#include <vector>

#include "../../../Common/MathHelper.h"

class LabyrinthGen
{
public:
    static LabyrinthGen* Instance;

    LabyrinthGen()
    {
        if(Instance != nullptr)
        {
            delete this;
        }
        else
        {
            Instance = this;
        }
    }

    DirectX::XMFLOAT3 position = {};
    
private:
    
    int finalSize = 32;
    int randomPasses = 2;
    float powerTwoPassCoef = 0.2f;
    bool trueFractal = false;
    bool disableRandomPasses = false;
    bool disableRotations = false;
    // Broken //////////////////
    bool disableTransposes = true;
    ///////////////////////////
    bool disableDeadEndRemoval = true;
    int deadEndRemovalPasses = 1;
    std::vector<int> removableWalls;
    std::vector<bool> finalTiles;

public:
    // Initialize and start maze gen
    std::vector<DirectX::XMFLOAT3> Generate()
    {
        srand(time(NULL));
        removableWalls.clear();
        finalTiles.clear();
        
        std::vector<bool> firstTiles;
        firstTiles.reserve(16);
        
        for (int y = 0; y < 4; y++)
            for (int x = 0; x < 4; x++)
            {
                if(y % 2 == 0 || x % 2 == 0)
                    firstTiles.push_back(true);
                else
                    firstTiles.push_back(false);
            }
        
        std::vector<int> firstDoors = { 6, 9, 11, 14 };

        firstDoors.erase(firstDoors.begin() + MathHelper::Rand(0, 4));
        
        for (auto door : firstDoors)
            firstTiles[door] = false;

        std::vector<bool> fractalTiles = Fractal(firstTiles, 4);

        if (!trueFractal && !disableRandomPasses)
        {
            PowerTwoPass(&fractalTiles);
            RandomPass(&fractalTiles);
        }
        
        finalTiles.reserve((finalSize + 1) * (finalSize + 1));
        for(int y = 0; y < finalSize + 1; y++)
            for(int x = 0; x < finalSize + 1; x++)
                if((x == finalSize || y == finalSize) || fractalTiles[y * finalSize + x])
                    finalTiles.push_back(true);
                else
                    finalTiles.push_back(false);
        
        if(!trueFractal && !disableDeadEndRemoval)
            for (int i = 0; i < deadEndRemovalPasses; i++)
                DeadEndRemoval(&finalTiles);

        // adding this stuff for 
        finalTiles[1] = false;
        finalTiles[finalTiles.size() - 2] = false;

        std::vector<DirectX::XMFLOAT3> wallLocations;
        wallLocations.reserve((finalSize + 1) * (finalSize + 1));
        for(int y = 0; y < finalSize + 1; y++)
            for(int x = 0; x < finalSize + 1; x++)
            {
                if(finalTiles[y * (finalSize + 1) + x])
                    wallLocations.push_back({static_cast<float>(x * 4) + position.x ,2 + position.y, static_cast<float>(y * 4) + position.z});
            }
        
        return wallLocations;    
    }

private:
    
    // Utility functions for maze generation
    std::vector<bool> Fractal(std::vector<bool>& inPattern, int curSize)
    {
        if (inPattern.size() >= finalSize * finalSize)
            return inPattern;
        
        std::vector<bool> outPattern;
        outPattern.reserve(curSize * curSize);
        
        for(int y = 0; y < curSize * 2; y++)
            for (int x = 0; x < curSize * 2; x++)
                outPattern.push_back(inPattern[(y % curSize) * curSize + x % curSize]);

        if (!trueFractal)
        {
            if (!disableTransposes)
            {
                if(RandomBool())
                    TransposeMatrix(outPattern, curSize + 1, curSize * 2, 1, curSize, curSize * 2);
                if(RandomBool())
                    TransposeMatrix(outPattern, 1, curSize, curSize + 1, curSize * 2, curSize * 2);
                if(RandomBool())
                    TransposeMatrix(outPattern, curSize + 1, curSize * 2, curSize + 1, curSize * 2, curSize * 2);                
            }
            
            if (!disableRotations)
            {
                for(int r = 0; r < MathHelper::Rand(0, 4); r++)
                    RotateMatrix(outPattern, curSize + 1, curSize * 2, 1, curSize, curSize * 2);            
                for(int r = 0; r < MathHelper::Rand(0, 4); r++)
                    RotateMatrix(outPattern, 1, curSize, curSize + 1, curSize * 2, curSize * 2);            
                for(int r = 0; r < MathHelper::Rand(0, 4); r++)
                    RotateMatrix(outPattern, curSize + 1, curSize * 2, curSize + 1, curSize * 2, curSize * 2);                
            }
        }
        
        int doorToFactal[] =
        {
            MathHelper::Rand(0, curSize / 2) * 2,
            MathHelper::Rand(0, curSize / 2) * 2,
            MathHelper::Rand(0, curSize / 2) * 2,
            MathHelper::Rand(0, curSize / 2) * 2
        };

        std::vector<int> indOfDoor = 
        {
            curSize * 2 * (doorToFactal[0] + 1) + curSize,
            curSize * 2 * curSize + curSize * 2 * (doorToFactal[1] + 1) + curSize,
            curSize * 2 * curSize + doorToFactal[2] + 1,
            curSize * 2 * curSize + curSize + doorToFactal[3] + 1 
        };
        
        if(trueFractal)
            indOfDoor.erase(indOfDoor.begin() + MathHelper::Rand(0,4));

        for(auto i : indOfDoor)
            outPattern[i] = false;
        
        return Fractal(outPattern, curSize * 2);
    }
    
    void PowerTwoPass(std::vector<bool>* tiles)
    {
        std::vector<bool> editList = *tiles;
        
        for(int y = 1; y < finalSize; y += 2)
            for (int x = 4; x < finalSize; x += 4)
            {
                auto xPower = MathHelper::NearestPow2(x) <= MathHelper::NearestPow2(finalSize - x) ? MathHelper::NearestPow2(x) :MathHelper::NearestPow2(finalSize - x);
                if (MathHelper::RandF() < static_cast<float>(xPower) * powerTwoPassCoef * 0.01)
                {
                    int i = y * finalSize + x;
                    editList[i] = false;
                }
            }
        
        for(int y = 4; y < finalSize; y += 4)
            for (int x = 1; x < finalSize; x += 2)
            {
                auto yPower = MathHelper::NearestPow2(y) <= MathHelper::NearestPow2(finalSize - y) ? MathHelper::NearestPow2(y) : MathHelper::NearestPow2(finalSize - y);
                if (MathHelper::RandF() < static_cast<float>(yPower) * powerTwoPassCoef * 0.01)
                {
                    editList[y * finalSize + x] = false;
                }
            }

        *tiles = editList;
    }
    
    void RandomPass(std::vector<bool>* tiles)
    {
        for(int y = 1; y < finalSize; y++ )
            for (int x = 1; x < finalSize; x += 2)
            {
                if((y % 2 == 1 && x + 1 < finalSize) && tiles->at(y * finalSize + x + 1))
                    removableWalls.push_back(y * finalSize + x + 1);
                else if (tiles->at(y * finalSize + x))
                    removableWalls.push_back(y * finalSize + x);
            }
        
        for (int i = 0; i < randomPasses; i++)
        {
            if (removableWalls.size() > 0)
            {
                int r = MathHelper::Rand(0, removableWalls.size());
                tiles->at(removableWalls[r]) = false;
                removableWalls.erase(removableWalls.begin() + r);
            }
        }
    }

    void DeadEndRemoval(std::vector<bool>* tiles)
    {
        for(int i = 0; i < 2; i++)
            for(int y = 1; y < finalSize + 1; y += 2)
                for (int x = 1 + (y + i) % 2 * 2 ; x < finalSize + 1; x += 4)
                {
                    bool up = !tiles->at((y + 1) * (finalSize + 1) + x);
                    bool down = !tiles->at((y - 1) * (finalSize + 1) + x);
                    bool left = !tiles->at(y * (finalSize + 1) + x - 1);
                    bool right = !tiles->at(y * (finalSize + 1) + x + 1);

                    int neighborCount = 0;

                    if (up)
                        neighborCount++;
                    if (down)
                        neighborCount++;
                    if (left)
                        neighborCount++;
                    if (right)
                        neighborCount++;

                    if (neighborCount == 1 && x + y != 2 && x + y != finalSize + finalSize - 2)
                    {
                        if (up)
                        {
                            tiles->at((y + 1) * (finalSize + 1) + x) = true;
                        }

                        if (down)
                        {
                            tiles->at((y - 1) * (finalSize + 1) + x) = true;
                        }

                        if (left)
                        {
                            tiles->at(y * (finalSize + 1) + x - 1) = true;
                        }

                        if (right)
                        {
                            tiles->at(y * (finalSize + 1) + x + 1) = true;
                        }
                        
                        tiles->at(y * (finalSize + 1) + x) = true;
                    }
                }
    }

    void TransposeMatrix(std::vector<bool>& tiles, int xStart, int xEnd, int yStart, int yEnd, int curSize)
    {
        for (int y = 0; y < curSize / 2 - 1; y++)
            for (int x = 0; x < curSize / 2 - y - 2; x++)
            {
                std::swap(tiles[(yEnd - x - 1) * curSize + xEnd - y - 1], tiles[(y + yStart) * curSize + x + xStart]);
                //bool a = tiles[(yEnd - x - 1) * curSize + xEnd - y - 1];
                //bool b = tiles[(y + yStart) * curSize + x + xStart];
                //tiles[(yEnd - x - 1) * curSize + xEnd - y - 1] = b;
                //tiles[(y + yStart) * curSize + x + xStart] = a;
                //(tiles[(yEnd - x - 1) * curSize + xEnd - y - 1], tiles[(y + yStart) * curSize + x + xStart]) 
                //    = (tiles[(y + yStart) * curSize + x + xStart], tiles[(yEnd - x - 1) * curSize + xEnd - y - 1]);
            }
    }

    void RotateMatrix(std::vector<bool>& tiles, int xStart, int xEnd, int yStart, int yEnd, int curSize)
    {
        std::vector<bool> newTiles;
        for(auto t : tiles)
        {
            newTiles.push_back(t);
        }
        
        for (int y = 0; y < curSize / 2 - 1; y++)
            for (int x = 0; x <curSize / 2 - 1; x++)
            {
                newTiles[(yEnd - x - 1) * curSize + y + xStart] = tiles[(yStart + y) * curSize + x + xStart];
            }
                
        for (int i = 0; i < tiles.size(); i++)
            tiles[i] = newTiles[i];
    }
    
    bool RandomBool()
    {
        return MathHelper::Rand(0, 2) % 2 == 1;
    }
    
};