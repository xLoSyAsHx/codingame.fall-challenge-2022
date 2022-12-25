#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <queue>

#include <random>
#include <numeric>
#include <algorithm>

//using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int g_mapWidth = 0, g_mapHeight = 0;

struct Pos {
    int x = 0, y = 0;
    bool isValid = false;

    Pos operator+(const Pos& p) { return Pos{x + p.x, y + p.y}; }
    Pos operator-(const Pos& p) { return Pos{x - p.x, y - p.y}; }
    bool operator==(const Pos& p) { return x == p.x && y == p.y; }
    bool operator!=(const Pos& p) { return x != p.x || y != p.y; }
    bool operator<(const Pos& p) { return x < p.x || (x == p.x && y < p.y); } // X coordinate has higher priority
    friend std::ostream& operator<<(std::ostream& os, const Pos& p) { os << " { x = " << p.x << "; y = " << p.y << "; isValid == " << p.isValid << " } "; }

    // Some approximation
    int distanceTo(Pos p) { return abs(x - p.x) + abs(y - p.y); }
};

#define DBG_MSG_V(v) { std::cerr << "MESSAGE: "#v" = " << v << std::endl; }
#define DBG_MSG2_V(v1, v2) { std::cerr << "MESSAGE: v1 = " << v1 << "; v2 = " << v2 << std::endl; }
#define DBG_MSG3_V(v1, v2, v3) { std::cerr << "MESSAGE: v1 = " << v1 << "; v2 = " << v2 << "; " << v3 <<  std::endl; }

#define DBG_MSG_ARR_V(v) { std::cerr << "MESSAGE: "#v" = { "; for (auto& el : v) std::cerr << el << "; "; std::cerr << std::endl; }
#define DBG_MSG_ARR_OF_PTR_V(v) { std::cerr << "MESSAGE: "#v" = { "; for (auto el : v) std::cerr << *el << "; "; std::cerr << std::endl; }

#define DBG_MSG_STR(s) { std::cerr << "MESSAGE: " << s << std::endl; }

#define DBG_MSG_PTR(v) { std::cerr << "MESSAGE: "#v" = " << *v << std::endl; }


struct MapCell {
    Pos p;

    int scrap_amount = 0;
    int owner = 0; // 1 = me, 0 = enemy, -1 = neutral
    int units = 0;
    int recycler = 0;

    // debug
    int can_build = 0;
    int can_spawn = 0;
    int in_range_of_recycler = 0;

    // calc constants
    int recycleProfit = -1;

    // Global ptr to map
    static std::vector<std::vector<MapCell>>& map;

    void FillFromConsole() {
        std::cin >> scrap_amount >> owner >> units >> recycler >> can_build >> can_spawn >> in_range_of_recycler; std::cin.ignore();
        recycleProfit = -1;
    }

    inline bool isMy()      const { return owner ==  1; }
    inline bool isEnemy()   const { return owner ==  0; }
    inline bool isNobodys() const { return owner == -1; }

    inline bool isValid()   const { return p.isValid && scrap_amount; }

    Pos getTop()    { Pos nP {p.x, p.y - 1}; if (nP.y >= 0)          nP.isValid = true; return nP; }
    Pos getBottom() { Pos nP {p.x, p.y + 1}; if (nP.y < g_mapHeight) nP.isValid = true; return nP; }
    Pos getLeft()   { Pos nP {p.x - 1, p.y}; if (nP.x >= 0)          nP.isValid = true; return nP; }
    Pos getRight()  { Pos nP {p.x + 1, p.y}; if (nP.x < g_mapWidth)  nP.isValid = true; return nP; }

    std::vector<MapCell*> getCellNeighbours() {
        std::vector<MapCell*> res = {};

        if (Pos p = getTop();    p.isValid && map[p.y][p.x].scrap_amount) res.emplace_back(&map[p.y][p.x]);
        if (Pos p = getBottom(); p.isValid && map[p.y][p.x].scrap_amount) res.emplace_back(&map[p.y][p.x]);
        if (Pos p = getLeft();   p.isValid && map[p.y][p.x].scrap_amount) res.emplace_back(&map[p.y][p.x]);
        if (Pos p = getRight();  p.isValid && map[p.y][p.x].scrap_amount) res.emplace_back(&map[p.y][p.x]);

        return res;
    }

    std::vector<Pos> getCellNeighboursPos() {
        std::vector<Pos> res = {};

        if (Pos p = getTop();    p.isValid && map[p.y][p.x].scrap_amount) res.emplace_back(p);
        if (Pos p = getBottom(); p.isValid && map[p.y][p.x].scrap_amount) res.emplace_back(p);
        if (Pos p = getLeft();   p.isValid && map[p.y][p.x].scrap_amount) res.emplace_back(p);
        if (Pos p = getRight();  p.isValid && map[p.y][p.x].scrap_amount) res.emplace_back(p);

        return res;
    }

    int getRecyclerProfit()
    {
        if (recycleProfit == -1)
        {
            recycleProfit = scrap_amount;

            std::vector<MapCell*> n = getCellNeighbours();
            std::for_each(n.begin(), n.end(), [this](MapCell* c) { recycleProfit += c->scrap_amount; });
        }
        return recycleProfit;
    }
};

struct GameInfo
{
    int currentTurn = -1;

    int my_matter  = 0;
    int opp_matter = 0;

    Pos prev_spawn {-1, -1};

    std::vector<std::vector<MapCell>> map = {};

    std::vector<MapCell*> myCells = {};
    std::vector<MapCell*> myCUnits = {};
    std::vector<MapCell*> enemyCUnits = {};
    int myCUnitsNum = 0;
    int enemyCUnitsNum = 0;

    int myRecycleNum = 0;
    int mySpawnNum = 0;
    int myMovedNum = 0;

    void ReadMatters() { std::cin >> my_matter >> opp_matter; std::cin.ignore(); }
    void ReadMapDimention() { std::cin >> g_mapWidth >> g_mapHeight; std::cin.ignore(); }
    void ReadCellsInput() {
        static bool isFirstCall = true;

        myCells.clear();
        myCUnits.clear();
        enemyCUnits.clear();
        myCUnitsNum = 0;
        enemyCUnitsNum = 0;
        myRecycleNum = 0;
        mySpawnNum = 0;
        myMovedNum = 0;
        

        if (isFirstCall)
        {
            map.resize(g_mapHeight);
            for (auto& row : map)
                row.resize(g_mapWidth);
        }

        for (int i = 0; i < g_mapHeight; i++) {
            for (int j = 0; j < g_mapWidth; j++) {
                auto pCell = &map[i][j];
                if (isFirstCall) {
                    pCell->p = { j, i };
                }

                pCell->p.isValid = false;
                pCell->FillFromConsole();
                if (pCell->scrap_amount)
                    pCell->p.isValid = true;

                if (pCell->isMy())
                {
                    myCells.emplace_back(pCell);

                    if (pCell->units)
                    {
                        myCUnits.emplace_back(pCell);
                        myCUnitsNum += pCell->units;
                    }
                    if (pCell->recycler)
                    {
                        myRecycleNum += 1;
                    }
                }
                else if (pCell->isEnemy() && pCell->units)
                {
                    enemyCUnits.emplace_back(pCell);
                    enemyCUnitsNum += pCell->units;
                }

                //DBG_MSG3_V(map[i][j].p, map[i][j].units, map[i][j].recycler);
            }
        }

        validate();

        isFirstCall = false;
        currentTurn++;
    }

    MapCell& getCell(Pos p)
    {
        if (p.isValid)
            return map[p.y][p.x];
        
        printf("ERROR line %d", __LINE__);
        return map[0][0];
    }

    /*
    bool canMoveOnDist(MapCell* cell, int stepsLeft);
    {
        for (auto neighbour : cell->getCellNeighbours())
        {
            if (canMoveOnDist(neighbour, stepsLeft - 1))
        }
    }
    */

    void validate()
    {


        // Remove enemy if we can't achieve it
        std::vector<MapCell*> toRemove = {};
        for (auto cUnit : enemyCUnits)
        {
            bool bRemove = true;
            for (auto neighbour : cUnit->getCellNeighbours())
                if (!neighbour->recycler)
                {
                    bRemove = false;
                    break;
                    /*
                    auto neighbour2 = neighbour->getCellNeighbours();
                    neighbour2.erase(std::find(neighbour2.begin(), neighbour2.end(), neighbour));
                    for (auto neighbour : neighbour2)
                    {
                        if (!neighbour->recycler)
                        {
                            bRemove = false;
                            goto nextStage;
                        }
                    }
                    */
                }
            

            if (bRemove)
            {
                toRemove.emplace_back(cUnit);
            }
        }

        for (auto el : toRemove)
        {
            DBG_MSG_STR("TO DELETE:");
            DBG_MSG_V(el);
            enemyCUnits.erase(std::find(enemyCUnits.begin(), enemyCUnits.end(), el));
        }



        // Remove my units
        std::vector<MapCell*> myToRemove = {};
        for (auto cUnit : myCUnits)
        {
            if (cUnit->getCellNeighbours().empty())
            {
                myToRemove.emplace_back(cUnit);
            }
        }
        
        for (auto el : myToRemove)
        {
            DBG_MSG_STR("TO DELETE MY:");
            DBG_MSG_V(el);
            myCUnits.erase(std::find(myCUnits.begin(), myCUnits.end(), el));
        }
    }

} GInf;

std::vector<std::vector<MapCell>>& MapCell::map = GInf.map;


struct Command {
    std::stringstream ss;
    bool bHasCommands = false;

    void Clear()  { ss.str(""); bHasCommands = false; }
    void Submit() { std::cout << ss.str() << std::endl; }

    void AddBuildRec(Pos p)                    { bHasCommands = true; ss << "BUILD " << p.x << ' ' << p.y << ';'; GInf.my_matter -= 1; }
    void AddMove(int amount, Pos from, Pos to) { bHasCommands = true; ss << "MOVE "  << amount << ' ' << from.x << ' ' << from.y << ' ' << to.x << ' ' << to.y << ';'; GInf.myMovedNum += amount;}
    void AddSpawn(int amount, Pos p)           { bHasCommands = true; ss << "SPAWN " << amount << ' ' << p.x << ' ' << p.y << ';';                                     GInf.mySpawnNum += amount; GInf.my_matter -= amount; }
    void AddWait()                             { bHasCommands = true; ss << "WAIT"; }
};

struct ProfitCell {
    int profit;
    MapCell* cell;
};
//std::array<ProfitCell, 3> findBestRecycler()
ProfitCell findBestRecycler()
{
    std::array<ProfitCell, 3> res = {};

    auto cmp = [](ProfitCell left, ProfitCell right) { return left.profit > right.profit; }; // Lower priority - first
    std::priority_queue<ProfitCell, std::vector<ProfitCell>, decltype(cmp)> res_queue(cmp);

    // Find all unique poses where player can build a recycler
    std::vector<Pos> uniquePoses = {};
    for (auto cell : GInf.myCells)
    {
        if (!cell->units && !cell->recycler)
            uniquePoses.emplace_back(cell->p);
    }

    for (auto cUnit : GInf.myCUnits)
    {
        int neighboursNum = cUnit->getCellNeighboursPos().size();
        DBG_MSG_V(neighboursNum);

        if (neighboursNum == 1)
        {
            if (auto it = std::find(uniquePoses.begin(), uniquePoses.end(), cUnit->getCellNeighboursPos()[0]); it != uniquePoses.end())
                uniquePoses.erase(it);
        }
    }
    // std::sort(uniquePoses.begin(), uniquePoses.end());
    // auto lastIt = std::unique(uniquePoses.begin(), uniquePoses.end());
    // uniquePoses.erase(lastIt, uniquePoses.end());

    // Add all unique poses to priority_queue and safe max size == 3
    std::for_each(uniquePoses.begin(), uniquePoses.end(), [&res_queue](const Pos& p) {
        auto& cell = GInf.getCell(p);
        int cellProfit = cell.getRecyclerProfit();

        //DBG_MSG2_V(p, &cell);

        res_queue.push(ProfitCell{ cellProfit, &cell });
        if (res_queue.size() > 3)
            res_queue.pop(); // remove el with the smallest priority
    });

    //DBG_MSG2_V(res_queue.top().cell, res_queue.top().cell);

    /*
    auto resIt = res.rend();
    for (int i = 0; i < res_queue.size(); ++i)
    {
        ++resIt;
        *resIt = res_queue.top();
        res_queue.pop();
    }
    */

    if (res_queue.empty())
        return {};

    return res_queue.top();
}

Pos findBestForSpawn()
{
    std::vector<Pos> uniquePoses = {};
    for (auto cell : GInf.myCells)
    {
        int neighboursNum = cell->getCellNeighbours().size();
        if (neighboursNum > 3 && !cell->recycler && cell->p != GInf.prev_spawn && cell->units < 3)
        {
            DBG_MSG_V(cell->p);
            GInf.prev_spawn = cell->p;
            return cell->p;
        }
    }
    return {-1, -1};
}

Pos findBestForResearchMove(MapCell* myCell)
{
    std::array<Pos, 4> candidates  = {
        myCell->getLeft(),
        myCell->getTop(),
        myCell->getBottom(),
        myCell->getRight(),
    };

    for (auto candidatePos : candidates)
    {
        if (candidatePos.isValid)
        {
            auto& candidateCell = GInf.getCell(candidatePos);
            if (candidateCell.isValid() && !candidateCell.isMy() && !candidateCell.recycler)
                return candidateCell.p;
        }
    }


    /*
    auto neigbourCells = myCell->getCellNeighbours();
    for (auto cell : neigbourCells)
    {
        if (cell->isValid() && !cell->isMy() && !cell->recycler && myCell->p < cell->p)
            return cell->p;
    }
    */


    return {};
}

Pos getUnitsEstCenterPos(std::vector<MapCell*>& units)
{
    if (units.empty())
        return {};

    Pos res {0, 0};
    std::for_each(units.begin(), units.end(), [&res](MapCell* c){ res = res + c->p; });
    
    res.x /= (int)units.size();
    res.y /= (int)units.size();

    return res;
}

Pos getNearestUnitToPos(std::vector<MapCell*>& units, Pos startP)
{
    if (units.empty())
        return {};

    Pos resP = units[0]->p;
    int smallestDist = startP.distanceTo(resP);

    for (auto unit : units)
        if (int dist = startP.distanceTo(unit->p); dist < smallestDist)
        {
            smallestDist = dist;
            resP = unit->p;
        }

    return resP;
}

Pos getMyUnitNearestToEnemyPos()
{
    // Find enemy center and select friend unit which is closest to it
    Pos enemyCenterEstPos = getUnitsEstCenterPos(GInf.enemyCUnits);
    return getNearestUnitToPos(GInf.myCUnits, enemyCenterEstPos);
}

bool willTurnToGrass(MapCell* cell)
{
    int currScrapAmount = cell->scrap_amount;
    for (auto neighbour : cell->getCellNeighbours())
    {
        if (neighbour->recycler)
            --currScrapAmount;
    }

    return currScrapAmount <= 0;
}

int getMaxToMove(MapCell* cell)
{
    return std::min(cell->units, GInf.myCUnitsNum - GInf.mySpawnNum - GInf.myMovedNum - 1);
}

Pos getNearestNooneCellPosToMove(MapCell* curCell)
{
    for (auto cell : curCell->getCellNeighbours())
    {
        if (cell->isNobodys() || (cell->isEnemy() && cell->recycler == 0))
            return cell->p;
        
        for (auto cell : cell->getCellNeighbours())
        {
            if (cell->isNobodys())
                return cell->p;
        }
    }

    return {};
}

int main()
{
    Pos fail_pos {-1, -1};
    
    Command command;
    
    MapCell* pMyCUnitsG1 = nullptr;

    Pos researcherPos = {};
    Pos additionalResearcherPos = {};

    // game loop
    GInf.ReadMapDimention();
    while (1) {
        GInf.ReadMatters();
        GInf.ReadCellsInput();
        command.Clear();

        if (additionalResearcherPos.isValid)
        {
            if (auto& cell = GInf.getCell(additionalResearcherPos); !cell.isMy() || cell.scrap_amount == 0)
                additionalResearcherPos = {};
        }

        bool is_spawn = false;

        DBG_MSG_V(GInf.currentTurn);
        std::cerr << "MESSAGE: GInf.myCUnits pos = ";
        for (auto el : GInf.myCUnits)
            std::cerr << el->p;
        std::cerr << std::endl;

        // DBG_MSG_V(GInf.myCUnits.size());
        // DBG_MSG_V(GInf.enemyCUnits.size());
        // DBG_MSG_V(GInf.myCUnitsNum);
        // DBG_MSG_V(GInf.enemyCUnitsNum);

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        pMyCUnitsG1 = &GInf.getCell(getMyUnitNearestToEnemyPos());
        DBG_MSG_V(pMyCUnitsG1->p);
        
        Pos nearestEnemyPos = getNearestUnitToPos(GInf.enemyCUnits, pMyCUnitsG1->p);
        DBG_MSG_V(nearestEnemyPos);


        if (GInf.myCUnitsNum < 2)
        {
            std::vector<MapCell*> randSpawn;
            size_t nelems = 4;
            std::sample(GInf.myCells.begin(), GInf.myCells.end(), std::back_inserter(randSpawn),
                nelems,
                std::mt19937{std::random_device{}()});

            for (auto cell : randSpawn)
            {
                if (cell->can_spawn)
                {
                    command.AddSpawn(GInf.my_matter / 10, cell->p);
                    command.Submit();
                    continue;
                }
            }
        }

        // command.AddMove(pMyCUnitsG1->units, pMyCUnitsG1->p, nearestEnemyPos);

        

        // Select Researcher
        /*
        if (!researcherPos.isValid)
        {
            int maxDistance = 0;
            for (auto el : GInf.myCUnits)
                if (int newDist = el->p.distanceTo(nearestEnemyPos); newDist > maxDistance)
                {
                    maxDistance = newDist;
                    researcherPos = el->p;
                }

        }
        MapCell* researcherCell = nullptr;

        if (GInf.myCUnits.size() > 1)
        {
            researcherCell = &GInf.getCell(researcherPos);
            GInf.myCUnits.erase(std::find(GInf.myCUnits.begin(), GInf.myCUnits.end(), researcherCell)); // remove researcher if there are another units
            DBG_MSG_V(researcherCell->p);
        }
        else
        {
            DBG_MSG_STR("NO Researcher");
        }

        DBG_MSG_STR("-------- Researcher select part finished --------");
        */

        if (GInf.myRecycleNum < 1 && GInf.my_matter >= 10)
        {
            ProfitCell cellForRecycler = findBestRecycler();
            if (cellForRecycler.cell == nullptr)
            {
                DBG_MSG_STR("cellForRecycler == nullptr");
            }
            else if (GInf.my_matter < 30)
            {
                auto cell_to_build = cellForRecycler.cell;
                DBG_MSG_V(cell_to_build->p);
                if (cell_to_build->can_build)
                {
                    command.AddBuildRec(cell_to_build->p);
                    GInf.my_matter -= 10;
                }
            }
        }

        // Spawn part
        while (true)
        {
            int myUnitsNum = GInf.myCUnitsNum;

            std::vector<MapCell*> randSpawn;
            if (GInf.my_matter >= 50)
            {
                size_t nelems = 3;
                std::sample(GInf.myCells.begin(), GInf.myCells.end(), std::back_inserter(randSpawn),
                    nelems,
                    std::mt19937{std::random_device{}()});
            }
            else if (GInf.my_matter >= 30)
            {
                size_t nelems = 1;
                std::sample(GInf.myCells.begin(), GInf.myCells.end(), std::back_inserter(randSpawn),
                    nelems,
                    std::mt19937{std::random_device{}()});
            }
            DBG_MSG_V(randSpawn.size());
            for (auto cell : randSpawn)
                if (cell->can_spawn)
                {
                    command.AddSpawn(1, cell->p);
                    is_spawn = true;
                    myUnitsNum += 1;
                }


            if (GInf.myCUnitsNum - GInf.mySpawnNum > 1 && myUnitsNum < 1.3*double(GInf.enemyCUnitsNum) && GInf.my_matter >= 30)
            {
                auto best_for_spawn = findBestForSpawn();
                if (best_for_spawn != fail_pos)
                {
                    command.AddSpawn(1, best_for_spawn);
                    is_spawn = true;
                    myUnitsNum += 1;
                }
                else
                {
                    //command.AddSpawn(1, pMyCUnitsG1->p);
                    //is_spawn = true;
                    //myUnitsNum += 1;
                    std::vector<MapCell*> randSpawn;
                    if (GInf.my_matter >= 50)
                    {
                        size_t nelems = 3;
                        std::sample(GInf.myCells.begin(), GInf.myCells.end(), std::back_inserter(randSpawn),
                            nelems,
                            std::mt19937{std::random_device{}()});
                    }
                    else if (GInf.my_matter >= 30)
                    {
                        size_t nelems = 1;
                        std::sample(GInf.myCells.begin(), GInf.myCells.end(), std::back_inserter(randSpawn),
                            nelems,
                            std::mt19937{std::random_device{}()});
                    }
                    for (auto cell : randSpawn)
                        if (cell->can_spawn)
                        {
                            command.AddSpawn(1, cell->p);
                            is_spawn = true;
                            myUnitsNum += 1;
                        }

                }
            }
            else
            {
                break;
            }
        }

        if (!is_spawn && GInf.my_matter >= 10)
        {
            command.AddSpawn(1, pMyCUnitsG1->p);
            is_spawn = true;
        }
        DBG_MSG_STR("-------- Spawn part  finished--------");

        // Global movement
        // 1) Check if recycler may destroy units
        // 2) Firstly - unite into one unit; Secondly - move; Repeat
        for (int i = 0; i < GInf.myCUnits.size(); i++)
        {
            auto myCell = GInf.myCUnits[i];
            if (willTurnToGrass(myCell))
            {
                // Firstly try to move to enemy direction
                Pos enemyDirection = nearestEnemyPos - myCell->p;
                Pos posToMove = {};
                if (enemyDirection.x > 0) { posToMove = myCell->getRight(); }
                else if (enemyDirection.x < 0) { posToMove = myCell->getLeft(); }
                else if (enemyDirection.y > 0) { posToMove = myCell->getTop(); }
                else if (enemyDirection.y < 0) { posToMove = myCell->getBottom(); }
                
                auto cellToMove = &GInf.getCell(posToMove);
                if (posToMove.isValid && cellToMove->scrap_amount && !willTurnToGrass(cellToMove))
                {
                    auto numUnitsToMove = getMaxToMove(myCell);
                    if (numUnitsToMove > 0)
                    {
                        command.AddMove(numUnitsToMove, myCell->p, cellToMove->p);

                        // Units already moved and can not do anything else. Lets just remove them
                        GInf.myCUnits.erase(GInf.myCUnits.begin() + i);
                        continue;
                    }
                }

                for (auto neighbour : myCell->getCellNeighbours())
                {
                    if (!neighbour->recycler && !willTurnToGrass(neighbour))
                    {
                        auto numUnitsToMove = getMaxToMove(myCell);
                        if (numUnitsToMove > 0)
                        {
                            command.AddMove(numUnitsToMove, myCell->p, neighbour->p);
                        }
                    }
                }

                // Units already moved and can not do anything else. Lets just remove them
                GInf.myCUnits.erase(GInf.myCUnits.begin() + i);
            }
        }
        
        DBG_MSG_STR("myCUnits to move:");
        DBG_MSG_V(GInf.myCUnits.size());

        if (GInf.enemyCUnits.size() > 0)
        {
            if (GInf.currentTurn % 2 == 0 && GInf.currentTurn < 10)
            {
                // unite into one unit
                for (int i = 0; i < std::max((int)GInf.myCUnits.size() - 1, 1); i++)
                {
                    auto cell_to_move = GInf.myCUnits[i];
                    auto numUnitsToMove = getMaxToMove(cell_to_move);
                    if (cell_to_move->p != pMyCUnitsG1->p)
                        command.AddMove(numUnitsToMove, cell_to_move->p, pMyCUnitsG1->p);
                }
            }
            else
            {
                DBG_MSG_V(additionalResearcherPos);

                // Additional rand research
                if (GInf.currentTurn % 5 == 4)
                {
                    std::vector<MapCell*> randResearchers;
                    std::sample(GInf.myCells.begin(), GInf.myCells.end() - 1, std::back_inserter(randResearchers),
                        1,
                        std::mt19937{std::random_device{}()});
                    
                    if (!randResearchers.empty())
                    {
                        auto randResearchCell = randResearchers[0];
                        Pos xxxxxxxxxxx_randResearcherPos = randResearchCell->p;
                        DBG_MSG_V(xxxxxxxxxxx_randResearcherPos);

                        auto numUnitsToMove = getMaxToMove(randResearchCell);
                        if (numUnitsToMove > 3)
                        {
                            numUnitsToMove = std::max(1, (int)(numUnitsToMove * 0.1));

                            Pos p = getNearestNooneCellPosToMove(randResearchCell);
                            if (p.isValid)
                            {
                                DBG_MSG_STR("============ AdditionalResearcher will be");
                                DBG_MSG_V(p);
                                command.AddMove(numUnitsToMove, randResearchCell->p, p);
                                randResearchCell->units -= numUnitsToMove;
                                additionalResearcherPos = p;
                            }
                        }
                    }
                }
                else if (additionalResearcherPos.isValid)
                {
                    auto researcherCell = &GInf.getCell(additionalResearcherPos);
                    auto numUnitsToMove = getMaxToMove(researcherCell);
                    if (numUnitsToMove > 5)
                    {
                        numUnitsToMove = std::max(1, (int)(numUnitsToMove * 0.1));

                        Pos p = getNearestNooneCellPosToMove(researcherCell);
                        if (p.isValid)
                        {
                            command.AddMove(numUnitsToMove, researcherCell->p, p);
                            researcherCell->units -= numUnitsToMove;
                            additionalResearcherPos = p;
                        }
                    }
                }

                for (int i = 0; i < std::max((int)GInf.myCUnits.size() - 1, 1); i++)
                {
                    auto cell_to_move = GInf.myCUnits[i];
                    auto numUnitsToMove = getMaxToMove(cell_to_move);
                    //if (cell_to_move->p != cell_to_build->p &&
                    //if (!(cell_to_move->p == pMyCUnitsG1->p))

                    Pos nearestEnemyPos = getNearestUnitToPos(GInf.enemyCUnits, cell_to_move->p);
                    command.AddMove(numUnitsToMove, cell_to_move->p, nearestEnemyPos);
                }
            }
        }
        DBG_MSG_STR("-------- Global movement  finished--------");

        // Research part
        if (GInf.myCUnits.size() > 1)
        {
            auto researcherCell = GInf.myCUnits.back();
            auto best_for_move = findBestForResearchMove(researcherCell);
            if (best_for_move.isValid && getMaxToMove(researcherCell) != 0)
            {
                command.AddMove(researcherCell->units, researcherCell->p, best_for_move);
                researcherPos = best_for_move;
            }
        }
            
        DBG_MSG_STR("-------- Research part  finished--------");
      
        if (!command.bHasCommands)
            command.AddWait();

        command.Submit();
    }
}