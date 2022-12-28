#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <queue>

#include <iomanip>
#include <memory>
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
    int x = -1, y = -1;
    bool isValid = false;

    Pos operator+(const Pos& p) { return Pos{x + p.x, y + p.y}; }
    Pos operator-(const Pos& p) { return Pos{x - p.x, y - p.y}; }
    bool operator==(const Pos& p) { return x == p.x && y == p.y; }
    bool operator!=(const Pos& p) { return x != p.x || y != p.y; }
    bool operator<(const Pos& p) { return x < p.x || (x == p.x && y < p.y); } // X coordinate has higher priority
    
    friend bool operator==(const Pos& lhd, const Pos& rhd) { return lhd.x == rhd.x && lhd.y == rhd.y; }
    friend std::ostream& operator<<(std::ostream& os, const Pos& p) { os << " { x = " << p.x << "; y = " << p.y << "; isValid == " << p.isValid << " } "; return os; }

    // Some approximation
    int distanceTo(Pos p) { return abs(x - p.x) + abs(y - p.y); }

    void invalidate() { x = y = -1; isValid = false; };

    bool isInsideMap() { return  x >= 0 && y >= 0 && x < g_mapWidth && y < g_mapHeight; }
};

#define DBG_MSG_V(msg, v) { std::cerr << msg << " " << v << std::endl; }
#define DBG_MSG_V2(msg, v1, v2) { std::cerr << msg << " " << v1 << " " << v2 << std::endl; }
#define DBG_MSG_V3(msg, v1, v2, v3) { std::cerr << msg << " " << v1 << " " << v2 << " " << v3 << std::endl; }

#define DBG_V(v) { std::cerr << ""#v" = " << v << std::endl; }
#define DBG_V2(v1, v2) { std::cerr << "v1 = " << v1 << "; v2 = " << v2 << std::endl; }
#define DBG_V3(v1, v2, v3) { std::cerr << "v1 = " << v1 << "; v2 = " << v2 << "; " << v3 <<  std::endl; }

#define DBG_MSG_ARR_V(v) { std::cerr << ""#v" = { "; for (auto& el : v) std::cerr << el << "; "; std::cerr << std::endl; }
#define DBG_MSG_ARR_OF_PTR_V(v) { std::cerr << ""#v" = { "; for (auto el : v) std::cerr << *el << "; "; std::cerr << std::endl; }

#define DBG_MSG_STR(s) { std::cerr << s << std::endl; }

#define DBG_MSG_PTR(v) { std::cerr << ""#v" = " << *v << std::endl; }

#define DBG_MAP_CELLS(v) { std::cerr << ""#v" = ["; for (auto& el : v) std::cerr << el->p; std::cerr << "]" << std::endl; }
#define DBG_MAP_POSES(v) { std::cerr << ""#v" = ["; for (auto& el : v) std::cerr << el;    std::cerr << "]" << std::endl; }

struct MapCell {
    Pos p;

    int scrap_amount = 0;
    int owner = 0; // 1 = me, 0 = enemy, -1 = neutral
    int units = 0;
    int recycler = 0;
    bool recyclerNextStep = 0;

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
        recyclerNextStep = 0;
    }

    inline bool isMy()        const { return owner ==  1; }
    inline bool isEnemy()     const { return owner ==  0; }
    inline bool isEnemyUnit() const { return isEnemy() && units > 0; }
    inline bool isNobodys()   const { return owner == -1; }
    inline bool hasUnitsRec() const { return units || recycler; }


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
            std::vector<MapCell*> n = getCellNeighbours();
            recycleProfit = std::accumulate(n.begin(), n.end(), scrap_amount, [](int acc, MapCell* rhd) { return acc + rhd->scrap_amount; });
        }
        return recycleProfit;
    }
};

struct ProfitCell {
    int profit;
    MapCell* cell;
};

struct GameInfo
{
    std::mt19937 random_device{std::random_device{}()};

    int currentTurn = -1;

    int my_matter  = 0;
    int opp_matter = 0;

    Pos prev_spawn {-1, -1};

    std::vector<std::vector<MapCell>> map = {};

    const int WA_INVALID = -1;        // To mark cell which is grass or recycler
    const int WA_ENEMY = 500;         // To mark enemy wave
    const int WA_NOT_PROCESSED = 999; // To mark cell which is not processed
    std::vector<std::vector<int>> WAmap = {};

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
            WAmap.resize(g_mapHeight);
            for (auto& row : map)
                row.resize(g_mapWidth);
            
            for (auto& row : WAmap)
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

                WAmap[i][j] = pCell->recycler || pCell->scrap_amount == 0 ? WA_INVALID : WA_NOT_PROCESSED;
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

        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ERROR GetCell!" << std::endl;
        return map[0][0];
    }

    bool isValid(Pos p)
    {
        if (p.x >= 0 && p.x < g_mapWidth && p.y >= 0 && p.y < g_mapHeight)
        {
            return getCell(p).isValid();
        }

        return false;
    }

    bool isMyValid(Pos p)
    {
        if (p.x >= 0 && p.x < g_mapWidth && p.y >= 0 && p.y < g_mapHeight)
        {
            const auto& cell = getCell(p);
            return cell.isValid() && cell.isMy();
        }

        return false;
    }

    void validate()
    {
        // TODO: Use wave-algorithm with depth 7-15. If my unit inside separated area - need to fully expance it

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
                }

            if (bRemove)
            {
                toRemove.emplace_back(cUnit);
            }
        }

        for (auto el : toRemove)
        {
            DBG_MSG_STR("TO DELETE:");
            DBG_V(el);
            enemyCUnits.erase(std::find(enemyCUnits.begin(), enemyCUnits.end(), el));
        }

        // Remove my units if they can't move.
        // TODO: Need to safe this cells to not spawn new units into it
        std::vector<MapCell*> myToRemove = {};
        for (auto cell : myCells)
        {
            if (cell->getCellNeighbours().empty())
            {
                myToRemove.emplace_back(cell);
            }
        }
        
        for (auto el : myToRemove)
        {
            DBG_MSG_STR("TO DELETE MY:");
            DBG_V(el);
            myCells.erase(std::find(myCells.begin(), myCells.end(), el));
        }

        DBG_MSG_STR("-------- Validation part finished--------");
    }

} GInf;

std::vector<std::vector<MapCell>>& MapCell::map = GInf.map;


class Command {
    std::stringstream ss;
    std::stringstream errSS;
    bool bEmpty = true;

public:
    bool empty() const { return bEmpty; }

    void Clear()  { ss.str(""); errSS.str(""); bEmpty = true; }
    void Submit() { std::cerr << errSS.str()<< std::endl; std::cout << ss.str() << std::endl; }

    void AddBuildRec(Pos p)                              { bEmpty = false; ss << "BUILD " << p.x << ' ' << p.y << ';'; GInf.my_matter -= 10; GInf.getCell(p).recyclerNextStep = 1; }
    void AddMove(int amount, Pos from, Pos to, int line) { bEmpty = false; ss << "MOVE "  << amount << ' ' << from.x << ' ' << from.y << ' ' << to.x << ' ' << to.y << ';'; GInf.myMovedNum += amount;
                                                                        errSS << "MOVE "  << amount << ' ' << from.x << ' ' << from.y << ' ' << to.x << ' ' << to.y << " l=" << line << "; ";}
    void AddSpawn(int amount, Pos p, int line)           { bEmpty = false; ss << "SPAWN " << amount << ' ' << p.x << ' ' << p.y << ';';                                     GInf.mySpawnNum += 1; GInf.my_matter -= amount * 10;
                                                                        errSS << "SPAWN " << amount << ' ' << p.x << ' ' << p.y << " l=" << line << "; "; }
    void AddWait()                                       { bEmpty = false; ss << "WAIT"; }
} command;


bool willCellTurnToGrass(MapCell* cell)
{
    int currScrapAmount = cell->scrap_amount;
    for (auto neighbour : cell->getCellNeighbours())
    {
        if (neighbour->recycler)
            --currScrapAmount;
    }

    return currScrapAmount <= 0;
}

bool hasEnemyNeighbours(MapCell* cell)
{
    for (auto neighbour : cell->getCellNeighbours())
    {
        if (neighbour->isEnemyUnit())
            return true;
    }

    return false;
}

int getEnemyNeighboursCount_D1(MapCell* cell)
{
    // Wave depth == 1

    int numEnemies = 0;
    for (auto neighbour : cell->getCellNeighbours())
    {
        if (neighbour->isEnemyUnit())
            numEnemies += neighbour->units;
    }

    return numEnemies;
}

int getEnemyNeighboursCount(MapCell* cell)
{
    // Wave depth == 2

    int numEnemies = 0;
    for (auto neighbour : cell->getCellNeighbours())
    {
        if (neighbour->isEnemyUnit())
            numEnemies += neighbour->units;

        for (auto neighbour2 : neighbour->getCellNeighbours())
        {
            if (neighbour2->p != neighbour->p && neighbour2->isEnemyUnit())
                numEnemies += neighbour2->units;
        }
    }

    return numEnemies;
}

// TODO: Do not use this dummy function
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

ProfitCell findBestRecycler()
{
    // Find all unique poses where player can build a recycler
    std::vector<Pos> uniquePoses = {};
    int curMaxScrap = 0;
    for (auto cell : GInf.myCells)
    {
        if (!cell->hasUnitsRec())
            uniquePoses.emplace_back(cell->p);
    }

    for (auto cUnit : GInf.myCUnits)
    {
        size_t neighboursNum = cUnit->getCellNeighboursPos().size();
        if (neighboursNum == 1)
        {
            if (auto it = std::find(uniquePoses.begin(), uniquePoses.end(), cUnit->getCellNeighboursPos()[0]); it != uniquePoses.end())
                uniquePoses.erase(it);
        }
    }

    // Sort elements by profit amount
    std::sort(uniquePoses.begin(), uniquePoses.end(), [](Pos& lhd, Pos& rhd) {
        return GInf.getCell(lhd).getRecyclerProfit() > GInf.getCell(rhd).getRecyclerProfit();
    });

    if (uniquePoses.empty())
        return {};

    auto pCell = &GInf.getCell(uniquePoses.front());
    return { pCell->getRecyclerProfit(), pCell };
}

struct SpawnRecomendation
{
    Pos p;
    int units;
};
SpawnRecomendation findBestForSpawn()
{
    /*
     * Best for spawn search steps in priority order
     *
     * 1) If there are enemies near my units in 2-cells radius - spawn as much as possible to protect (if will not turn to grass)
     * 2) Find normal cell which will not turn into grass
     * 3) Find my unit nearest to enemy and spawn here
    */

   // ----- 1 -----
    for (auto pBestForSpawnSell : GInf.myCUnits)
    {
        if (int numEnemies = getEnemyNeighboursCount(pBestForSpawnSell); numEnemies && !willCellTurnToGrass(pBestForSpawnSell))
        {
            DBG_V(pBestForSpawnSell->p);

            int toSpawn = numEnemies - pBestForSpawnSell->units;
            /*
            if (toSpawn < 0)
            {
                toSpawn = 1;
            }
            else
            */
            {
                toSpawn = std::min(GInf.my_matter / 10, getEnemyNeighboursCount(pBestForSpawnSell));
            }

            return { pBestForSpawnSell->p, toSpawn };
        }
    }

    // ----- 2 -----
    if (GInf.myCUnitsNum < 1.3*double(GInf.enemyCUnitsNum))
    {
        for (auto pBestForSpawnSell : GInf.myCells)
        {
            size_t neighboursNum = pBestForSpawnSell->getCellNeighbours().size();
            if (neighboursNum > 3 && !pBestForSpawnSell->can_spawn && pBestForSpawnSell->p != GInf.prev_spawn && !willCellTurnToGrass(pBestForSpawnSell))
            {
                DBG_V(pBestForSpawnSell->p);
                GInf.prev_spawn = pBestForSpawnSell->p;
                return { pBestForSpawnSell->p, 1 };
            }
        }
    }

    // ----- 3 -----
    /*
    Pos p = getMyUnitNearestToEnemyPos();
    if (p.isValid)
    {
        return { p, 1 };
    }
    */

    return {};
}

Pos findBestForResearchMove(MapCell* myCell)
{
    // TODO: Need to try to move away ffrom enemies
    // Example: if my{ 5, 0 }, nearest_enemy{ 3, 0 }, need to move Right or Bottom

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
            if (candidateCell.isValid() && !candidateCell.isMy() && !candidateCell.recycler && !candidateCell.recyclerNextStep)
                return candidateCell.p;
        }
    }

    return {};
}

int getMaxToMove(MapCell* cell)
{
    // TODO: maybe need to refine

    // If algorithm spawn units or moved them - we need to take it into account
    return std::min(cell->units, GInf.myCUnitsNum - GInf.mySpawnNum - GInf.myMovedNum);
}

Pos getNearestNooneCellPosToMove(MapCell* curCell)
{
    // TODO: use wave-algorithm with depth 5-7

    // Depth 1
    for (auto cell : curCell->getCellNeighbours())
    {
        if (cell->isNobodys() || (cell->isEnemy() && !cell->recycler && !cell->recyclerNextStep))
            return cell->p;
    }

    // Depth 2
    for (auto cell : curCell->getCellNeighbours())
    {
        for (auto cell2 : cell->getCellNeighbours())
        {
            if (cell2->isNobodys() || (cell2->isEnemy() && !cell2->recycler && !cell->recyclerNextStep))
                return cell2->p;
        }
    }

    return {};
}

/*
struct PosRoute {
    Pos p;
    std::vector<Pos>
};
*/
struct Poses_DestFirst {
    Pos pDest;
    Pos firstToMove;
};
Poses_DestFirst getNearest_Enemy_CellPosToMove(MapCell* curCell)
{
    Poses_DestFirst res{};
    // TODO: use wave-algorithm with depth 5-7

    // Depth 1
    for (auto cell : curCell->getCellNeighbours())
    {
        if (cell->isEnemy() && !cell->recycler && !cell->recyclerNextStep)
        {
            res.firstToMove = res.pDest = cell->p;
            return res;
        }
    }

    // Depth 2
    for (auto cell : curCell->getCellNeighbours())
    {
        res.firstToMove = cell->p;
        for (auto cell2 : cell->getCellNeighbours())
        {
            if (cell2->isEnemy() && !cell2->recycler && !cell2->recyclerNextStep)
            {
                res.pDest = cell2->p;
                return res;
            }
        }
    }

    return {};
}
Poses_DestFirst getNearest_Noone_CellPosToMove(MapCell* curCell)
{
    Poses_DestFirst res{};
    // TODO: use wave-algorithm with depth 5-7

    // Depth 1
    for (auto cell : curCell->getCellNeighbours())
    {
        if (cell->isNobodys())
        {
            res.firstToMove = res.pDest = cell->p;
            return res;
        }
    }

    // Depth 2
    for (auto cell : curCell->getCellNeighbours())
    {
        res.firstToMove = cell->p;
        for (auto cell2 : cell->getCellNeighbours())
        {
            if (cell2->isNobodys())
            {
                res.pDest = cell2->p;
                return res;
            }
        }
    }

    return {};
}

Poses_DestFirst getNearest_EnemyOrNoone_CellPosToMove(MapCell* curCell)
{
    auto res = getNearest_Enemy_CellPosToMove(curCell);
    if (!res.pDest.isValid)
    {
        res = getNearest_Noone_CellPosToMove(curCell);
    }

    return res;
}

void getRandomCells(std::vector<MapCell*> &v, std::vector<MapCell*> &out, size_t nelems)
{
    std::sample(v.begin(), v.end(), std::back_inserter(out),
        nelems,
        GInf.random_device);
}

class StrategyInterface
{
public:
    void Execute()
    {
        if (!bInitialized)
        {
            OnInitialize();
            if (!bInitialized)
                return; // Stop execute strategy. Initialize failed
        }

        OnTurnStart();
        OnRecyclerPart();
        OnSpawnPart();
        OnMovementPart();
        OnSpecialUnitsPart();
    }

protected:
    virtual void OnInitialize() { bInitialized = true; }
    virtual void OnTurnStart() = 0;
    virtual void OnRecyclerPart() = 0;
    virtual void OnSpawnPart() = 0;
    virtual void OnMovementPart() = 0;
    virtual void OnSpecialUnitsPart() = 0;

    bool bInitialized = false;
};

class Strategy_1Hunter_1Researcher_1Invader final : public StrategyInterface
{
protected:

    void OnTurnStart()
    {
        if (!GInf.isMyValid(researcherPos)) researcherPos.invalidate();
        if (!GInf.isMyValid(invaderPos))    invaderPos.invalidate();

        if (researcherPos.isValid && hasEnemyNeighbours(&GInf.getCell(researcherPos)))
        {
            DBG_MSG_V("WARN - Researcher in dangerous - behave like normal unit", researcherPos);
            researcherPos.invalidate();
        }

        // TODO: If invader is important - do not remove it
        if (invaderPos.isValid && hasEnemyNeighbours(&GInf.getCell(invaderPos)))
        {
            DBG_MSG_V("WARN - Invider in dangerous - behave like normal unit", invaderPos);
            invaderPos.invalidate();
        }

        // If number of units too small - spend all matters to spawn units
        // TODO: Maybe need to spawn near the enemy
        if (GInf.myCUnitsNum < 2)
        {
            std::vector<MapCell*> randSpawn;
            getRandomCells(GInf.myCells, randSpawn, 10);

            for (auto cell : randSpawn)
            {
                if (cell->can_spawn)
                {
                    command.AddSpawn(GInf.my_matter / 10, cell->p, __LINE__);
                    command.Submit();
                    continue;
                }
            }
        }
    }

    void OnRecyclerPart()
    {
        DBG_MSG_STR("-------- Recycler part start --------");
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
                DBG_V(cell_to_build->p);
                if (cell_to_build->can_build)
                {
                    command.AddBuildRec(cell_to_build->p);
                    GInf.my_matter -= 10;
                }
            }
        }
    }

    void OnSpawnPart()
    {
        DBG_MSG_STR("-------- Spawn part start --------");
        {
            if (GInf.my_matter >= 20)
            {
                auto spawnRec = findBestForSpawn();
                DBG_V(spawnRec.p);
                if (spawnRec.p.isValid)
                {
                    command.AddSpawn(spawnRec.units, spawnRec.p, __LINE__);
                }
            }
        }
    }

    void OnMovementPart()
    {
        // Global movement
        DBG_MSG_STR("-------- Global movement start --------");
        // 1) Check if recycler may destroy units. If so - move them to safe cell
        // 2) Move all units to nearest enemy

        // ============================================= Global movement 1 =============================================
        for (int i = 0; i < GInf.myCUnits.size(); i++)
        {
            auto myCell = GInf.myCUnits[i];
            if (willCellTurnToGrass(myCell))
            {
                // 1 - Find nearest enemy pos to current unit
                Pos nearestEnemyPos = getNearestUnitToPos(GInf.enemyCUnits, myCell->p);

                // 2 - Try to move to enemy direction
                Pos enemyDirection = nearestEnemyPos - myCell->p;
                Pos posToMove{};
                if (enemyDirection.x > 0)      { posToMove = myCell->getRight(); }
                else if (enemyDirection.x < 0) { posToMove = myCell->getLeft(); }
                else if (enemyDirection.y > 0) { posToMove = myCell->getTop(); }
                else if (enemyDirection.y < 0) { posToMove = myCell->getBottom(); }
                
                auto cellToMove = &GInf.getCell(posToMove);
                if (posToMove.isValid && cellToMove->scrap_amount && !willCellTurnToGrass(cellToMove))
                {
                    auto numUnitsToMove = getMaxToMove(myCell);
                    if (numUnitsToMove > 0)
                    {
                        command.AddMove(numUnitsToMove, myCell->p, cellToMove->p, __LINE__);

                        // Units already moved and can not do anything else. Lets just remove them
                        GInf.myCUnits.erase(GInf.myCUnits.begin() + i);
                        continue;
                    }
                }

                // 3 - Can't move to 'cellToMove' try to move somewhere
                for (auto neighbour : myCell->getCellNeighbours())
                {
                    if (!neighbour->recycler && !neighbour->recyclerNextStep && !willCellTurnToGrass(neighbour))
                    {
                        auto numUnitsToMove = getMaxToMove(myCell);
                        if (numUnitsToMove > 0)
                        {
                            command.AddMove(numUnitsToMove, myCell->p, neighbour->p, __LINE__);

                            // Units already moved and can not do anything else. Lets just remove them
                            GInf.myCUnits.erase(GInf.myCUnits.begin() + i);
                        }
                    }
                }
            }
        }
        
        DBG_MSG_STR("After willCellTurnToGrass checks:");
        DBG_V(GInf.myCUnits.size());

        if (GInf.enemyCUnits.size() > 0)
        {
            // TODO: dummy logic - need to remove
            if (GInf.currentTurn % 2 == 0 && GInf.currentTurn < 10)
            {
                auto pMyCUnitsG1 = &GInf.getCell(getMyUnitNearestToEnemyPos());

                // unite into one unit: move all units to pMyCUnitsG1
                for (int i = 0; i < std::max((int)GInf.myCUnits.size() - 1, 1); i++)
                {
                    auto cell_to_move = GInf.myCUnits[i];

                    // Do not move researcher and invader
                    if (researcherPos.isValid && cell_to_move->p == researcherPos ||
                        invaderPos.isValid    && cell_to_move->p == invaderPos)
                        continue;

                    auto numUnitsToMove = getMaxToMove(cell_to_move);
                    if (cell_to_move->p != pMyCUnitsG1->p)
                    {
                        command.AddMove(numUnitsToMove, cell_to_move->p, pMyCUnitsG1->p, __LINE__);
                    }
                }
            }
            else
            {
                // ============================================= Global movement 2 =============================================
                for (int i = 0; i < std::max((int)GInf.myCUnits.size(), 1); i++)
                {
                    auto cell_to_move = GInf.myCUnits[i];

                    // Do not move researcher and invader
                    if (researcherPos.isValid && cell_to_move->p == researcherPos ||
                        invaderPos.isValid    && cell_to_move->p == invaderPos)
                        continue;

                    auto numUnitsToMove = getMaxToMove(cell_to_move);
                    if (numUnitsToMove > 0)
                    {
                        Pos nearestEnemyPos = getNearestUnitToPos(GInf.enemyCUnits, cell_to_move->p);
                        command.AddMove(numUnitsToMove, cell_to_move->p, nearestEnemyPos, __LINE__);
                    }
                }
            }
        }
    }
    
    void OnSpecialUnitsPart()
    {
        // Research part
        DBG_MSG_STR("-------- Research part start --------");
        if (!researcherPos.isValid && GInf.my_matter >= 10)
        {
            Pos enemyCenterPos = getUnitsEstCenterPos(GInf.enemyCUnits);
            int maxDistance = 0;
            for (auto myCell : GInf.myCells)
            {
                if (myCell->hasUnitsRec())
                    continue;

                int dist = myCell->p.distanceTo(enemyCenterPos);
                if (dist > maxDistance)
                {
                    auto[pDest, pFirstStep] = getNearest_EnemyOrNoone_CellPosToMove(myCell);
                    if (pDest.isValid)
                    {
                        maxDistance = dist;
                        researcherPos = myCell->p;
                        DBG_MSG_V("Nearest_NooneOrEnemy:", pDest);
                    }
                }
            }
            
            DBG_MSG_V("Spawn researcher at:", researcherPos);
            command.AddSpawn(1, researcherPos, __LINE__);
        }
        else if (researcherPos.isValid)
        {
            auto myResearcher = &GInf.getCell(researcherPos);
            auto[pNotMyDest, pFirstStep] = getNearest_EnemyOrNoone_CellPosToMove(myResearcher);
            if (pNotMyDest.isValid)
            {
                command.AddMove(myResearcher->units, myResearcher->p, pFirstStep, __LINE__);
                researcherPos = pFirstStep;
                DBG_MSG_V("Move researcher to:", pFirstStep);
            }
            else
            {
                researcherPos.invalidate();
            }
        }

        // Invader part
        DBG_MSG_STR("-------- Invader part start --------");
        if (!invaderPos.isValid && GInf.my_matter >= 10 && GInf.myCUnitsNum + GInf.my_matter/10 - 2 > GInf.enemyCUnitsNum + GInf.opp_matter/10)
        {
            int maxDistance = 0; // global max distance to nearest enemy
            for (auto myCell : GInf.myCells)
            {
                if (myCell->hasUnitsRec())
                    continue;

                int cellMinDistToEnemy = INT32_MAX;
                std::for_each(GInf.enemyCUnits.begin(), GInf.enemyCUnits.end(), [&cellMinDistToEnemy, myCell](MapCell* c) { cellMinDistToEnemy = std::min(cellMinDistToEnemy, myCell->p.distanceTo(c->p)); });
                if (cellMinDistToEnemy > maxDistance)
                {
                    auto[pDest, pFirstStep] = getNearest_Enemy_CellPosToMove(myCell);
                    if (pDest.isValid)
                    {
                        maxDistance = cellMinDistToEnemy;
                        invaderPos = myCell->p;
                        DBG_MSG_V("Nearest_Enemy:", pDest);
                    }
                }
            }
            
            DBG_MSG_V("Spawn invader at:", invaderPos);
            command.AddSpawn(1, invaderPos, __LINE__);
        }
        else if (invaderPos.isValid)
        {
            DBG_MSG_V("Invader pos:", invaderPos);
            auto myInvader = &GInf.getCell(invaderPos);
            auto[pDest, pFirstStep] = getNearest_Enemy_CellPosToMove(myInvader);
            if (pDest.isValid)
            {
                command.AddMove(myInvader->units, myInvader->p, pFirstStep, __LINE__);
                researcherPos = pFirstStep;
                DBG_MSG_V("Move invader to:", pFirstStep);
            }
            else
            {
                invaderPos.invalidate();
                DBG_MSG_V("Invalidate invader since no enemy cells around:", invaderPos);
            }
        }

        DBG_MSG_STR("--------------------------------------");
    }

private:
    Pos researcherPos{};
    Pos invaderPos{};
};

class Strategy_Butterfly_1Middle_2Wings final : public StrategyInterface
{
protected:
    virtual void OnInitialize()
    {
        // Map for wave algorithm
        std::vector<std::vector<int>> map;
        map.resize(GInf.WAmap.size());
        for (int i = 0; i < map.size(); ++i)
            map[i] = GInf.WAmap[i];

        // Opponent on the right or on the left
        Pos myCenter = getUnitsEstCenterPos(GInf.myCUnits);
        Pos enemyCenter = getUnitsEstCenterPos(GInf.enemyCUnits);
        if ((enemyCenter - myCenter).x > 0) bOppRight = true;
        else                                bOppRight = false;

        // Select reseachers
        for (auto pCell : GInf.myCUnits)
        {
            Pos p = pCell->p;
            // Top
            if (p.y < rTop_Pos.y ||
                (p.y == rTop_Pos.y && (bOppRight ? p.x > rTop_Pos.x : p.x < rTop_Pos.x))) rTop_Pos = p;
            
            // Bottom
            if (p.y > rBot_Pos.y ||
                (p.y == rBot_Pos.y && (bOppRight ? p.x > rBot_Pos.x : p.x < rBot_Pos.x))) rBot_Pos = p;
        }

        if (Pos{0, 0}.distanceTo(enemyCenter - myCenter) > myCenter.y)
        {
            // Launch wave to find best target
            Pos enemyTop = getNearestUnitToPos(GInf.enemyCUnits, rTop_Pos);
            Pos enemyBot = getNearestUnitToPos(GInf.enemyCUnits, rBot_Pos);
            LaunchWaveAlgorithm(enemyTop.y, enemyTop.x, &map, 20, enemyBot.y, false, Dir::TOP); // Top enemy est
            LaunchWaveAlgorithm(enemyBot.y, enemyBot.x, &map, 20, enemyTop.y, false, Dir::BOT); // Bot enemy est

            LaunchWaveAlgorithm(rTop_Pos.y, rTop_Pos.x, &map, 20, rBot_Pos.y, true, Dir::TOP); // Top reseacher
            LaunchWaveAlgorithm(rBot_Pos.y, rBot_Pos.x, &map, 20, rTop_Pos.y, true, Dir::BOT); // Bot reseacher

            for (int y = 0; y < g_mapHeight; ++y)
            {
                std::cerr << std::endl;
                for (int x = 0; x < g_mapWidth; ++x)
                    std::cerr << ' ' << std::setw(3) << map[y][x];
            }
            std::cerr << std::endl;

            // Determine best position for reseachers to stop
            Pos rTop_Target{};
            Pos rBot_Target{};
            for (int y = 1; y >= 0; --y)
            {
                int x = 0;
                for (; x < map[y].size() - 1; ++x)
                    if (map[y][x] != GInf.WA_INVALID && map[y][x] < GInf.WA_ENEMY && map[y][x+1] >= GInf.WA_ENEMY)
                        break;

                if (x != map[y].size() - 1)
                {
                    rTop_Target = Pos{ x - 1, y, true };
                    break;
                }
            }

            for (int y = map.size() - 2;  y < map.size(); ++y)
            {
                int x = 0;
                for (; x < map[y].size() - 1; ++x)
                    if (map[y][x] != GInf.WA_INVALID && map[y][x] < GInf.WA_ENEMY && map[y][x+1] >= GInf.WA_ENEMY)
                        break;

                if (x != map[y].size() - 1)
                {
                    rBot_Target = Pos{ x - 1, y, true };
                    break;
                }
            }
            
            DBG_MSG_V2("Reseacher targets: ", rTop_Target, rBot_Target);
            if (!rTop_Target.isValid || !rBot_Target.isValid)
            {
                DBG_MSG_STR("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Failed to initialize Butterfly strategy. Can't calculate  reseacher targets");
                return;
            }

            Pos rTop_TargetOrig = rTop_Target;
            Pos rBot_TargetOrig = rBot_Target;
            if (rTop_Target.y != 0 && GInf.isValid(Pos {rTop_Target.x, 0,             true})) rTop_Target = {rTop_Target.x, 0};
            if (rBot_Target.y != 0 && GInf.isValid(Pos {rBot_Target.x, g_mapHeight-1, true})) rBot_Target = {rBot_Target.x, g_mapHeight-1};

            // Build route to targets
            rTop_Route = BuildRouteFromWave(map, rTop_Pos, rTop_Target, Dir::RIGHT);
            rBot_Route = BuildRouteFromWave(map, rBot_Pos, rBot_Target, Dir::RIGHT);
            rTop_Target = rTop_TargetOrig;
            rBot_Target = rBot_TargetOrig;
 
            // Find best positions for recyclers
            auto v = GetBestRecyclersToEliminateRoute(rTop_Route, true, 1);
            recyclers.insert(recyclers.end(), v.begin(), v.end());

            v = GetBestRecyclersToEliminateRoute(rBot_Route, false, 1);
            recyclers.insert(recyclers.end(), v.begin(), v.end());

            // Add extra actions
            rTop_Route.emplace_back(Pos{0, 0, true});
            rBot_Route.emplace_back(Pos{0, g_mapHeight-1, true});

            DBG_MSG_ARR_V(rTop_Route);
            DBG_MSG_ARR_V(rBot_Route);
        }

        bInitialized = true;
    }
    virtual void OnTurnStart()
    {

    }
    virtual void OnRecyclerPart()
    {
        auto canBuildRec = [](const Pos& p) { auto pCell = &GInf.getCell(p); return pCell->isMy() && pCell->units == 0 && !pCell->recycler && !pCell->recyclerNextStep; };

        auto it = std::find_if(recyclers.begin(), recyclers.end(), canBuildRec);
        while (it != recyclers.end())
        {
            command.AddBuildRec(*it);
            it = std::find_if(it, recyclers.end(), canBuildRec);
        }
    }
    virtual void OnSpawnPart()
    {
        // Check, need to stop protect route
        bool bAllRecyclersFinishedWork = true;
        for (auto p : recyclers)
            if (GInf.getCell(p).scrap_amount != 0)
            {
                bAllRecyclersFinishedWork = false;
                break;
            }
        if (bAllRecyclersFinishedWork)
            bNeedToProtectRoute = false;

        // Route protection logic - just spawn units to prevent invade
        if (bNeedToProtectRoute)
        {
            for (const auto& route : {rTop_Route, rBot_Route})
                for (auto p : route)
                {
                    auto pCell = &GInf.getCell(p);
                    if (int enemies = getEnemyNeighboursCount_D1(pCell); enemies > pCell->units && !pCell->recycler && !pCell->recyclerNextStep && !willCellTurnToGrass(pCell))
                    {
                        command.AddSpawn(enemies - pCell->units, pCell->p, __LINE__);
                    }
                }
        }
    }
    virtual void OnMovementPart() {}
    virtual void OnSpecialUnitsPart()
    {
        if (GInf.currentTurn < rTop_Route.size())
        {
            auto rTop_Cell = &GInf.getCell(rTop_Pos);
            command.AddMove(rTop_Cell->units, rTop_Cell->p, rTop_Route[GInf.currentTurn], __LINE__); rTop_Pos = rTop_Route[GInf.currentTurn];
        }
        
        if (GInf.currentTurn < rBot_Route.size())
        {
            auto rBot_Cell = &GInf.getCell(rBot_Pos);
            command.AddMove(rBot_Cell->units, rBot_Cell->p, rBot_Route[GInf.currentTurn], __LINE__); rBot_Pos = rBot_Route[GInf.currentTurn];
        }
    }

private:

    // WA parameters
    enum Dir { TOP, BOT, RIGHT, LEFT };
    std::vector<std::vector<int>> *WA_map = nullptr;
    int  WA_depth = 0;
    int  WA_extYLimit = 0;
    bool WA_bMy = false;
    Dir  WA_FirstDir = Dir::TOP;
    void LaunchWaveAlgorithm(int y, int x, std::vector<std::vector<int>> *map, int depth, int extYLimit, bool bMy, Dir FirstDir)
    {
        WA_map = map; WA_depth = depth; WA_extYLimit = extYLimit; WA_bMy = bMy; WA_FirstDir = FirstDir;
        //DBG_MSG_v2("LaunchWaveAlgorithm_rec", y, x);
        LaunchWaveAlgorithm_rec(y, x, 0);
    }

    void LaunchWaveAlgorithm_rec(int y, int x, int curDepth)
    {
        if (!Pos{x, y}.isInsideMap() || y == WA_extYLimit)
            return;
        
        int& map_v_ref = (*WA_map)[y][x];
        int unitCurDepth = WA_bMy ? curDepth : curDepth + GInf.WA_ENEMY;
        if (map_v_ref == GInf.WA_INVALID || (map_v_ref <= unitCurDepth))
            return;
        
        // Found enemy
        if (WA_bMy && map_v_ref > GInf.WA_ENEMY)
        {
            int enemyRealDepth = map_v_ref - GInf.WA_ENEMY;
            if (unitCurDepth >= enemyRealDepth)
                return;
        }
        
        map_v_ref = unitCurDepth;

        ++curDepth;
        if (curDepth == WA_depth)
            return;
        
        if (WA_FirstDir == Dir::TOP)
        {
            LaunchWaveAlgorithm_rec(y-1, x,   curDepth);
            LaunchWaveAlgorithm_rec(y,   x+1, curDepth);
            LaunchWaveAlgorithm_rec(y+1, x,   curDepth);
            LaunchWaveAlgorithm_rec(y,   x-1, curDepth);
        }
        else
        {
            LaunchWaveAlgorithm_rec(y+1, x,   curDepth);
            LaunchWaveAlgorithm_rec(y,   x+1, curDepth);
            LaunchWaveAlgorithm_rec(y-1, x,   curDepth);
            LaunchWaveAlgorithm_rec(y,   x-1, curDepth);
        }
    }

    std::vector<Pos> BuildRouteFromWave(std::vector<std::vector<int>> &map, Pos cur, Pos target, Dir firstDir)
    {
        if (!cur.isInsideMap() || !target.isInsideMap() || map[cur.y][cur.x] == GInf.WA_NOT_PROCESSED || map[target.y][target.x] == GInf.WA_NOT_PROCESSED)
        {
            DBG_MSG_V2("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Failed to initialize Butterfly strategy. Can't BuildRouteFromWave", cur, target);
            return {};
        }

        // Will find route from end to start
        std::swap(cur, target);

        std::vector<Pos> route{};
        int curDepth = map[cur.y][cur.x];
        while (cur != target)
        {
            cur.isValid = true;
            route.emplace_back(cur);

            Pos nextPos={};
            if (firstDir == Dir::RIGHT)
            {
                for (const Pos& step : {Pos{1,0}, Pos{0,1}, Pos{0,-1}, Pos{-1,0}})
                {
                    nextPos = cur + step;
                    if (nextPos.isInsideMap() && map[nextPos.y][nextPos.x] == curDepth - 1)
                        break;
                }
            }
            else if (firstDir == Dir::LEFT)
            {
                for (const Pos& step : {Pos{-1,0}, Pos{0,1}, Pos{0,-1}, Pos{1,0}})
                {
                    nextPos = cur + step;
                    if (nextPos.isInsideMap() && map[nextPos.y][nextPos.x] == curDepth - 1)
                        break;
                }
            }

            --curDepth;
            cur = nextPos;
        }

        std::reverse(route.begin(), route.end());
        return route;
    }

    std::vector<Pos> GetBestRecyclersToEliminateRoute(std::vector<Pos> route, bool bEnemyBottom, int offset)
    {
            // find cells through which enemy can invade
            std::vector<Pos> cellsToProtect;
            std::vector<Pos> recyclers;
            for (int i = offset; i < route.size(); ++i)
            {
                auto c = &GInf.getCell(route[i]);
                Pos p1 = bEnemyBottom ? c->getBottom() : c->getTop();
                Pos p2 = bOppRight    ? c->getRight()  : c->getLeft();
                DBG_V3(c->p, p1, p2);
                // cell valid and not in my route
                if (GInf.isValid(p1) && std::count(route.begin(), route.end(), p1) == 0 ||
                    GInf.isValid(p2) && std::count(route.begin(), route.end(), p2) == 0)
                    cellsToProtect.emplace_back(route[i]);
            }
            DBG_MSG_ARR_V(cellsToProtect);

            int curLen = 0;
            for (int i = 0; i < cellsToProtect.size(); ++i)
            {
                auto p = cellsToProtect[i];
                if (curLen == 1 && p.distanceTo(cellsToProtect[i-1]) != 1) // new cell is far from previous. Add recycler
                {
                    DBG_MSG_V3("recyclers C1: ", i, curLen, p);
                    recyclers.emplace_back(cellsToProtect[i-1]);
                    curLen = 0;
                }
                else if (curLen == 2)
                {
                    DBG_MSG_V3("recyclers C2: {i, curLen, p} ", i, curLen, p);
                    int profit1 = GInf.getCell(cellsToProtect[i-2]).getRecyclerProfit();
                    int profit2 = GInf.getCell(cellsToProtect[i-1]).getRecyclerProfit();
                    int profit3 = GInf.getCell(cellsToProtect[i-0]).getRecyclerProfit();
                    if (profit1 > profit2) recyclers.emplace_back(cellsToProtect[i-2]);
                    else                   recyclers.emplace_back(cellsToProtect[i-1]);
                    curLen = 0;

                    if (p.distanceTo(cellsToProtect[i-1]) == 1 && profit2 >= profit3 && profit2 >= profit1)
                        i++; // Skip this cell. It will processed by above recycler
                }

                if (i == cellsToProtect.size() - 1)
                {
                    DBG_MSG_V3("recyclers C4: {i, curLen, p} ", i, curLen, p);
                    if (curLen == 0) recyclers.emplace_back(cellsToProtect.back()); // previous cells already processed. Need to proceed only this one
                    else
                    {
                        int profit1 = GInf.getCell(cellsToProtect[i-1]).getRecyclerProfit();
                        int profit2 = GInf.getCell(cellsToProtect[i-0]).getRecyclerProfit();
                        
                        if (profit1 > profit2) recyclers.emplace_back(cellsToProtect[i-1]);
                        else                   recyclers.emplace_back(cellsToProtect[i-0]);
                    }

                }
                ++curLen;
            }
            DBG_MSG_ARR_V(recyclers);
            return recyclers;
    }

    bool bOppRight = false; // if true - my units on the left, enemy on the right
    bool bNeedToProtectRoute = true;

    // Reseachers
    Pos rTop_Pos{ INT32_MAX, INT32_MAX };
    Pos rBot_Pos{ -1, -1 };

    // Reseacher targets
    std::vector<Pos> rTop_Route;
    std::vector<Pos> rBot_Route;

    // Recyclers
    std::vector<Pos> recyclers;
};


int main()
{
    std::unique_ptr<StrategyInterface> strategyItf;

    // game loop
    GInf.ReadMapDimention();
    while (1) {
        GInf.ReadMatters();
        GInf.ReadCellsInput();
        command.Clear();

        DBG_V(GInf.currentTurn);
        DBG_MAP_CELLS(GInf.myCUnits);

        if (GInf.currentTurn == 0)
        {
            //strategyItf.reset(new Strategy_1Hunter_1Researcher_1Invader);
            strategyItf.reset(new Strategy_Butterfly_1Middle_2Wings);
        }
        
        strategyItf->Execute();
      
        if (command.empty())
            command.AddWait();

        command.Submit();
    }
}
