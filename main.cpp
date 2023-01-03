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
#define DBG_MSG_V2(msg, v1, v2) { std::cerr << msg << " "#v1 << ' ' << v1 << " " << #v2 << ' ' << v2 << std::endl; }
#define DBG_MSG_V3(msg, v1, v2, v3) { std::cerr << msg << " " << #v1 << ' ' << v1 << " " << #v2 << ' ' << v2 << " " << #v3 << ' ' << v3 << std::endl; }

#define DBG_V(v) { std::cerr << ""#v" = " << v << std::endl; }
#define DBG_V2(v1, v2) { std::cerr << " " << #v1 << ' ' << v1 << "; " << #v2 << ' ' << v2 << std::endl; }
#define DBG_V3(v1, v2, v3) { std::cerr << " " << #v1 << ' ' << v1 << ";  " << #v2 << ' ' <<  v2 << "; " << #v3 << ' ' << v3 <<  std::endl; }

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
    inline bool isMyUnit()    const { return isMy() && units > 0; }
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
            recycleProfit = scrap_amount;
            for (auto neighbour : n)
            {
                recycleProfit += std::min(scrap_amount, neighbour->scrap_amount);
            }
        }
        return recycleProfit;
    }
};

struct ProfitCell {
    int profit;
    MapCell* cell;
};

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

struct GameInfo
{
    std::mt19937 random_device{std::random_device{}()};

    int currentTurn = -1;

    int my_matter  = 0;
    int opp_matter = 0;

    bool bOppRight = false;
    bool bAsymmetricMap = false;

    Pos prev_spawn {-1, -1};

    std::vector<std::vector<MapCell>> map = {};

    const int WA_INVALID = -1;        // To mark cell which is grass or recycler
    const int WA_ENEMY = 500;         // To mark enemy wave
    const int WA_NOT_PROCESSED = 999; // To mark cell which is not processed
    std::vector<std::vector<int>> WAmap = {};

    std::vector<MapCell*> myCells = {};
    std::vector<MapCell*> myCUnits = {};
    std::vector<MapCell*> enemyCUnits = {};
    std::vector<MapCell*> enemyCells = {};
    int myCUnitsNum = 0;
    int enemyCUnitsNum = 0;
    int enemyRecycleNum = 0;

    int myRecycleNum = 0;
    int mySpawnNum = 0;
    int myMovedNum = 0;

    Pos initiaEstEnemyCenter{};
    int initialDistanceToEnemy = 0;

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
        enemyRecycleNum = 0;
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
                else if (pCell->isEnemy())
                {
                    enemyCells.emplace_back(pCell);
                    if (pCell->units)
                    {
                        enemyCUnits.emplace_back(pCell);
                        enemyCUnitsNum += pCell->units;
                    }

                    if (pCell->recycler)
                        enemyRecycleNum += 1;
                }

                WAmap[i][j] = pCell->recycler || pCell->scrap_amount == 0 ? WA_INVALID : WA_NOT_PROCESSED;
            }
        }

        if (isFirstCall)
        {
            initiaEstEnemyCenter = getUnitsEstCenterPos(enemyCUnits);
            initialDistanceToEnemy = getUnitsEstCenterPos(myCUnits).distanceTo(initiaEstEnemyCenter);
            DBG_V(initiaEstEnemyCenter);
            DBG_V(initialDistanceToEnemy);

            // Opponent on the right or on the left
            Pos myCenter = getUnitsEstCenterPos(myCUnits);
            Pos enemyCenter = getUnitsEstCenterPos(enemyCUnits);
            if ((enemyCenter - myCenter).x > 0) bOppRight = true;
            else                                bOppRight = false;

            DBG_V2(myCenter, enemyCenter);
            if (std::abs(enemyCenter.y - myCenter.y) > 1) bAsymmetricMap = true;
            else                                          bAsymmetricMap = false;
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
    std::stringstream errSS_Build, errSS_Spawn, errSS_Mpve;
     bool bEmpty = true;

 public:
     bool empty() const { return bEmpty; }

    void Clear()  { ss.str(""); errSS_Build.str(""); errSS_Spawn.str(""); errSS_Mpve.str(""); bEmpty = true; }
    void Submit() { std::cerr << errSS_Build.str()<< std::endl << errSS_Spawn.str()<< std::endl << errSS_Mpve.str()<< std::endl; std::cout << ss.str() << std::endl; }

    void AddBuildRec(Pos p, int l)                       { bEmpty = false; ss << "BUILD " << p.x << ' ' << p.y << ';'; GInf.my_matter -= 10; GInf.getCell(p).recyclerNextStep = 1;
                                                                  errSS_Build << "BUILD " << p.x << ' ' << p.y << "L " << l << "; ";}
     void AddMove(int amount, Pos from, Pos to, int line) { bEmpty = false; ss << "MOVE "  << amount << ' ' << from.x << ' ' << from.y << ' ' << to.x << ' ' << to.y << ';'; GInf.myMovedNum += amount;
                                                                   errSS_Mpve << "MOVE "  << amount << ' ' << from.x << ' ' << from.y << ' ' << to.x << ' ' << to.y << " l=" << line << "; ";}
     void AddSpawn(int amount, Pos p, int line)           { bEmpty = false; ss << "SPAWN " << amount << ' ' << p.x << ' ' << p.y << ';';                                     GInf.mySpawnNum += 1; GInf.my_matter -= amount * 10;
                                                                  errSS_Spawn << "SPAWN " << amount << ' ' << p.x << ' ' << p.y << " l=" << line << "; "; }
     void AddWait()                                       { bEmpty = false; ss << "WAIT"; }
 } command;


bool isOnEnemySide(MapCell* cell)
{
    if (GInf.bOppRight) return cell->p.x >= g_mapWidth/2;
    else                return cell->p.x <= g_mapWidth/2;
}

bool hasEnemyFreeCellNeighbours(MapCell* cell)
{
    for (auto neighbour : cell->getCellNeighbours())
    {
        if (neighbour->isEnemy() && ! neighbour->recycler && neighbour->units == 0)
            return true;
    }

    return false;
}

bool hasNobodyNeighbours(MapCell* cell)
{
    for (auto neighbour : cell->getCellNeighbours())
    {
        if (neighbour->isNobodys())
            return true;
    }

    return false;
}

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

bool hasRecyclerNeighbours(MapCell* pCell)
{
    for (auto neighbour : pCell->getCellNeighbours())
    {
        if (neighbour->recycler || neighbour->recyclerNextStep)
            return true;
    }

    return false;
}

bool hasEnemyUnitNeighbours(MapCell* cell)
{
    for (auto neighbour : cell->getCellNeighbours())
    {
        if (neighbour->isEnemyUnit())
            return true;
    }

    return false;
}

int getEnemyUnitNeighboursCount_D1(MapCell* cell)
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

int getEnemyUnitNeighboursCount_D2(MapCell* cell)
{
    // Wave depth == 2
    std::vector<MapCell*> neighbours;
    neighbours.reserve(14);

    for (auto neighbour : cell->getCellNeighbours())
    {
        if (!neighbour->isEnemyUnit())
            continue;

        neighbours.emplace_back(neighbour);
        for (auto neighbour2 : neighbour->getCellNeighbours())
        {
            if (!neighbour2->isEnemyUnit())
                continue;

            neighbours.emplace_back(neighbour2);
        }
    }

    neighbours.erase(std::unique(neighbours.begin(), neighbours.end()), neighbours.end());
    return std::accumulate(neighbours.begin(), neighbours.end(), 0, [](int u, MapCell* pCell) { return u + pCell->units; });
}

int getMyNeighboursCount(MapCell* cell)
{
    // Wave depth == 2
    std::vector<MapCell*> neighbours;
    neighbours.reserve(14);

    for (auto neighbour : cell->getCellNeighbours())
    {
        if (!neighbour->isEnemyUnit())
            continue;

        neighbours.emplace_back(neighbour);

        for (auto neighbour2 : neighbour->getCellNeighbours())
        {
            if (!neighbour2->isEnemyUnit())
                continue;

            neighbours.emplace_back(neighbour2);
        }
    }

    neighbours.erase(std::unique(neighbours.begin(), neighbours.end()), neighbours.end());
    return std::accumulate(neighbours.begin(), neighbours.end(), 0, [](int u, MapCell* pCell) { return u + pCell->units; });
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

std::vector<MapCell*> getN_NearestUnitToPos(std::vector<MapCell*>& units, Pos startP, int n)
{
    std::vector<MapCell*> poses(units);
    std::sort(poses.begin(), poses.end(), [&startP](MapCell* lhd, MapCell* rhd) { return lhd->p.distanceTo(startP) < rhd->p.distanceTo(startP); });

    poses.resize(n);
    return poses;
}

Pos getFarestUnitToPos(std::vector<MapCell*>& units, Pos startP)
{
    if (units.empty())
        return {};

    Pos resP = units[0]->p;
    int largestDist = startP.distanceTo(resP);

    for (auto unit : units)
        if (int dist = startP.distanceTo(unit->p); dist > largestDist)
        {
            largestDist = dist;
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
    Pos p{};
    int units{};
};
std::vector<SpawnRecomendation> findBestForSpawn()
{
    /*
     * Best for spawn search steps in priority order
     *
     * 1) If there are enemies near my units in 2-cells radius - spawn as much as possible to protect (if will not turn to grass)
    */
   std::vector<SpawnRecomendation> recomendations{};
   int matter = GInf.my_matter;
   
   // ----- 1 -----
    for (auto pBestForSpawnSell : GInf.myCUnits)
    {
        if (int numEnemies = getEnemyUnitNeighboursCount_D2(pBestForSpawnSell); numEnemies && !willCellTurnToGrass(pBestForSpawnSell)
            && pBestForSpawnSell->can_spawn && !pBestForSpawnSell->recyclerNextStep
            && std::count_if(recomendations.begin(), recomendations.end(), [cell = pBestForSpawnSell](auto r) { return cell->p == r.p; }) == 0)
        {
            if (matter < 10)
                break;

            int toSpawn = std::min(matter / 10, getEnemyUnitNeighboursCount_D2(pBestForSpawnSell) + 1);

            DBG_V3(pBestForSpawnSell->p, toSpawn, __LINE__);
            matter -= toSpawn;
            recomendations.emplace_back(SpawnRecomendation{ pBestForSpawnSell->p, toSpawn });
        }
    }
    
   // ----- 1.5 -----
   // Protect my cells
    for (auto pBestForSpawnSell : GInf.myCells)
    {
        if (int numEnemies = getEnemyUnitNeighboursCount_D1(pBestForSpawnSell); numEnemies && !willCellTurnToGrass(pBestForSpawnSell)
            && pBestForSpawnSell->can_spawn && !pBestForSpawnSell->recyclerNextStep
            && std::count_if(recomendations.begin(), recomendations.end(), [cell = pBestForSpawnSell](auto r) { return cell->p == r.p; }) == 0)
        {
            if (matter < 10)
                break;

            int toSpawn = std::min(matter / 10, getEnemyUnitNeighboursCount_D2(pBestForSpawnSell));

            DBG_V3(pBestForSpawnSell->p, toSpawn, __LINE__);
            matter -= toSpawn;
            recomendations.emplace_back(SpawnRecomendation{ pBestForSpawnSell->p, toSpawn });
        }
    }

    return recomendations;
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

struct Poses_DestFirst {
    Pos pDest;
    Pos firstToMove;
};
Poses_DestFirst getNearest_Enemy_CellPosToMove(MapCell* curCell)
{
    Poses_DestFirst res{};

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

Pos getBestReseacherToSpawnPos()
{
    Pos researcherPos{};
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
            }
        }
    }

    return researcherPos;
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

        if (researcherPos.isValid && hasEnemyUnitNeighbours(&GInf.getCell(researcherPos)))
        {
            DBG_MSG_V("WARN - Researcher in dangerous - behave like normal unit", researcherPos);
            researcherPos.invalidate();
        }

        // TODO: If invader is important - do not remove it
        if (invaderPos.isValid && hasEnemyUnitNeighbours(&GInf.getCell(invaderPos)))
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
                    command.AddBuildRec(cell_to_build->p, __LINE__);
                    GInf.my_matter -= 10;
                }
            }
        }
    }

    void OnSpawnPart()
    {
        DBG_MSG_STR("-------- Spawn part start --------");
        {
            if (GInf.my_matter >= 10)
            {
                for (auto& spawnRec : findBestForSpawn())
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
            researcherPos = getBestReseacherToSpawnPos();
            
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

        bOppRight = GInf.bOppRight;

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

        bInitialized = true;
    }
    virtual void OnTurnStart()
    {
        if (!GInf.isMyValid(researcherPos))    researcherPos.invalidate();
        if (researcherPos.isValid && hasEnemyUnitNeighbours(&GInf.getCell(researcherPos)))
        {
            DBG_MSG_V("WARN - Researcher in dangerous - behave like normal unit", researcherPos);
            researcherPos.invalidate();
        }

        if (!GInf.isMyValid(cellDestroyerPos))  cellDestroyerPos.invalidate();
        if (!GInf.isValid(cellDestroyerTarget)) cellDestroyerTarget.invalidate();


        if (GInf.currentTurn < 40 && (rTop_Pos.isValid || rBot_Pos.isValid))
        {
            // Map for wave algorithm
            std::vector<std::vector<int>> map;
            map.resize(GInf.WAmap.size());
            for (int i = 0; i < map.size(); ++i)
                map[i] = GInf.WAmap[i];
            
            Pos enemyTop = getNearestUnitToPos(GInf.enemyCUnits, Pos{rTop_Pos.x, 0});
            Pos enemyBot = getNearestUnitToPos(GInf.enemyCUnits, Pos{rBot_Pos.x, g_mapHeight-1});

            rTop_NextStep = GetNextButterflyWingStep(map, rTop_Pos, enemyTop, rBot_Pos.y, true);

            map.resize(GInf.WAmap.size());
            for (int i = 0; i < map.size(); ++i)
                map[i] = GInf.WAmap[i];
            rBot_NextStep = GetNextButterflyWingStep(map, rBot_Pos, enemyBot, rTop_Pos.y, false);

            DBG_V2(rTop_Pos, rBot_Pos);
            DBG_V2(rTop_NextStep, rBot_NextStep);

            if (rTop_NextStep.isValid) rTop_Route.emplace_back(rTop_NextStep);
            else                       rTop_Pos.invalidate();

            if (rBot_NextStep.isValid) rBot_Route.emplace_back(rBot_NextStep);
            else                       rBot_Pos.invalidate();
        }
    }
    virtual void OnRecyclerPart()
    {
        DBG_MSG_STR("#### OnRecyclerPart start");

        // Sort to extract far recyclers first
        std::sort(recyclers.begin(), recyclers.end(), [](Pos lhd, Pos rhd) { return GInf.bOppRight ? lhd.x > rhd.x : lhd.x < rhd.x; });

        auto canBuildRec = [](const Pos& p) { auto pCell = &GInf.getCell(p); return pCell->isMy() && pCell->units == 0 && !pCell->recycler && !pCell->recyclerNextStep; };
        auto it = std::find_if(recyclers.begin(), recyclers.end(), canBuildRec);
        while (it != recyclers.end() && GInf.myRecycleNum < 2)
        {
            command.AddBuildRec(*it, __LINE__);
            it = std::find_if(it, recyclers.end(), canBuildRec);
            break;
        }

        // Defence my cells
        for (auto pCell : GInf.myCells)
        {
            int numEnemies = getEnemyUnitNeighboursCount_D1(pCell);
            if (!pCell->recycler && !pCell->recyclerNextStep && pCell->can_build)
            {
                if (isOnEnemySide(pCell) && !hasRecyclerNeighbours(pCell) || numEnemies > 1)
                    command.AddBuildRec(pCell->p, __LINE__);
            }
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

        if (GInf.my_matter >= 10)
        {
            std::vector<Pos> alreadySwawned;
            for (auto p : rTop_Route)
            {
                auto pCell = &GInf.getCell(p);
                if (GInf.isMyValid(p) && hasEnemyUnitNeighbours(pCell) && !pCell->recycler && !pCell->recyclerNextStep)
                {
                    command.AddSpawn(getEnemyUnitNeighboursCount_D1(pCell), p, __LINE__);
                    alreadySwawned.emplace_back(p);
                }
            }
            
            for (auto p : rBot_Route)
            {
                auto pCell = &GInf.getCell(p);
                if (GInf.isMyValid(p) && hasEnemyUnitNeighbours(pCell) && !pCell->recycler && !pCell->recyclerNextStep)
                {
                    command.AddSpawn(getEnemyUnitNeighboursCount_D1(pCell), p, __LINE__);
                    alreadySwawned.emplace_back(p);
                }
            }

            for (auto& spawnRec : findBestForSpawn())
                if (spawnRec.p.isValid && std::find(alreadySwawned.begin(), alreadySwawned.end(), spawnRec.p) == alreadySwawned.end())
                {
                    command.AddSpawn(spawnRec.units, spawnRec.p, __LINE__);
                }
        }
    }
    virtual void OnMovementPart()
    {
        // Copied from base strategy
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
                DBG_MSG_V("Cell will turn into grass", myCell->p);
                DBG_MSG_V("potential pos to move ", cellToMove->p);
                if (posToMove.isValid && cellToMove->scrap_amount && !cellToMove->recycler &&!willCellTurnToGrass(cellToMove))
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
                command.AddMove(myCell->units, myCell->p, {g_mapWidth-1,g_mapHeight-1}, __LINE__);

                // Units already moved and can not do anything else. Lets just remove them
                GInf.myCUnits.erase(GInf.myCUnits.begin() + i);

            }
        }

        for (int i = 0; i < std::max((int)GInf.myCUnits.size(), 1); i++)
        {
            auto cell_to_move = GInf.myCUnits[i];

            // Do not move researcher and invader
            if (researcherPos.isValid    && cell_to_move->p == researcherPos ||
                cellDestroyerPos.isValid && cell_to_move->p == cellDestroyerPos)
                continue;

            bool bWingRobots = rTop_Pos.isValid && cell_to_move->p == rTop_Pos ||
                               rBot_Pos.isValid && cell_to_move->p == rBot_Pos;

            auto numUnitsToMove = getMaxToMove(cell_to_move);
            if (bWingRobots)
                --numUnitsToMove;

            if (numUnitsToMove > 0)
            {
                Pos nearestEnemyPos = getNearestUnitToPos(GInf.enemyCUnits, cell_to_move->p);
                Pos enemySubMyPos = nearestEnemyPos - cell_to_move->p;

                auto nearEnemyCell_toMove = getNearest_Enemy_CellPosToMove(cell_to_move); 
                auto nearEnemyNooneCell_toMove = getNearest_EnemyOrNoone_CellPosToMove(cell_to_move);

                bool bNoEnemyUnits_D1 = getEnemyUnitNeighboursCount_D1(cell_to_move) == 0;
                bool bCanProtectNearCell_D1 = false;
                for (auto pCell : cell_to_move->getCellNeighbours())
                    if (int numEnemies = getEnemyUnitNeighboursCount_D1(pCell);
                        pCell->isMy() && numEnemies > pCell->units && numEnemies - pCell->units <= cell_to_move->units)
                    {
                        bCanProtectNearCell_D1 = true;
                    }

                if (bNoEnemyUnits_D1 && bCanProtectNearCell_D1)
                {
                    DBG_V3("@@@@@@@@@@@ 0", cell_to_move->p, nearEnemyNooneCell_toMove.pDest);
                    int availableUnitsNum = cell_to_move->units;
                    for (auto pCell : cell_to_move->getCellNeighbours())
                    {
                        int numEnemies = getEnemyUnitNeighboursCount_D1(pCell);
                        int numUnitsToHelp = numEnemies - pCell->units;
                        if (pCell->isMy() && numEnemies > pCell->units && availableUnitsNum)
                        {
                            numUnitsToHelp = std::min(numUnitsToHelp, availableUnitsNum);
                            command.AddMove(numUnitsToHelp, cell_to_move->p, pCell->p, __LINE__);
                            availableUnitsNum -= numUnitsToHelp;
                        }
                    }
                }
                else if (isOnEnemySide(cell_to_move) && bNoEnemyUnits_D1 &&
                        (GInf.bOppRight ? GInf.isValid(cell_to_move->getRight()) : GInf.isValid(cell_to_move->getLeft()) ||
                        GInf.isValid(cell_to_move->getTop()) || GInf.isValid(cell_to_move->getBottom())))
                {
                    DBG_V3("@@@@@@@@@@@ 1", cell_to_move->p, nearEnemyNooneCell_toMove.pDest);
                    Pos enemySideDirection = GInf.bOppRight ? cell_to_move->getRight() : cell_to_move->getLeft();

                    if (GInf.isValid(enemySideDirection))          command.AddMove(cell_to_move->units, cell_to_move->p, enemySideDirection, __LINE__);
                    else if (GInf.isValid(cell_to_move->getTop())) command.AddMove(cell_to_move->units, cell_to_move->p, cell_to_move->getTop(), __LINE__);
                    else                                           command.AddMove(cell_to_move->units, cell_to_move->p, cell_to_move->getBottom(), __LINE__);
                }
                else if (cell_to_move->units > 1 && bNoEnemyUnits_D1 && (hasEnemyFreeCellNeighbours(cell_to_move) || hasNobodyNeighbours(cell_to_move)))
                {
                    DBG_V3("@@@@@@@@@@@ 2", cell_to_move->p, nearEnemyNooneCell_toMove.pDest);
                    for (auto pCell : cell_to_move->getCellNeighbours())
                        if (pCell->isNobodys() && numUnitsToMove > 0)
                        {
                            --numUnitsToMove;
                            command.AddMove(1, cell_to_move->p, pCell->p, __LINE__);
                        }
                    
                    for (auto pCell : cell_to_move->getCellNeighbours())
                        if (pCell->isEnemy() && numUnitsToMove > 0)
                        {
                            --numUnitsToMove;
                            command.AddMove(1, cell_to_move->p, pCell->p, __LINE__);
                        }
                }
                else if (cell_to_move->p.distanceTo(nearestEnemyPos) == 1)
                {
                    numUnitsToMove = std::max(numUnitsToMove - getEnemyUnitNeighboursCount_D1(cell_to_move), 0);
                    command.AddMove(numUnitsToMove, cell_to_move->p, nearestEnemyPos, __LINE__);
                }
                else if (Pos cornerP = Pos{cell_to_move->p.y + enemySubMyPos.y, cell_to_move->p.x};
                            std::abs(enemySubMyPos.x) == 1 && std::abs(enemySubMyPos.y) == 1 && // Enemy on corner and corner cell valid and not a recycler. Better to go top or bot
                            GInf.isValid(cornerP) && !GInf.getCell(cornerP).recycler && !GInf.getCell(cornerP).recyclerNextStep)
                {
                    enemySubMyPos.x = 0;
                    command.AddMove(numUnitsToMove, cell_to_move->p, cell_to_move->p + enemySubMyPos, __LINE__);
                }
                else if (cell_to_move->p.distanceTo(nearestEnemyPos) > 5 && isOnEnemySide(cell_to_move) &&
                        (nearEnemyCell_toMove.pDest.isValid || nearEnemyNooneCell_toMove.pDest.isValid)) // Enemy on corner. Better to go top or bot
                {
                    if (nearEnemyCell_toMove.pDest.isValid) command.AddMove(numUnitsToMove, cell_to_move->p, nearEnemyCell_toMove.firstToMove, __LINE__);
                    else                                    command.AddMove(numUnitsToMove, cell_to_move->p, nearEnemyNooneCell_toMove.firstToMove, __LINE__);
                }
                else
                    command.AddMove(numUnitsToMove, cell_to_move->p, nearestEnemyPos, __LINE__);
            }
        }

        DBG_MSG_V3("#### OnMovementPart end. ", GInf.myCUnitsNum, GInf.myMovedNum, GInf.mySpawnNum);
    }
    virtual void OnSpecialUnitsPart()
    {
        // Process moving by route
        if (rTop_NextStep.isValid)
        {
            auto rTop_Cell = &GInf.getCell(rTop_Pos);
            command.AddMove(1, rTop_Cell->p, rTop_NextStep, __LINE__);
            rTop_Pos = rTop_NextStep;
        }
        if (rBot_NextStep.isValid)
        {
            auto rBot_Cell = &GInf.getCell(rBot_Pos);
            command.AddMove(1, rBot_Cell->p, rBot_NextStep, __LINE__);
            rBot_Pos = rBot_NextStep;
        }

        // Research part from base strategy
        if (GInf.currentTurn >= 5 + GInf.initialDistanceToEnemy)
        {
            DBG_MSG_STR("-------- Research part start --------");
            if (!researcherPos.isValid && GInf.my_matter >= 10)
            {
                researcherPos = getBestReseacherToSpawnPos();
                
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
        LaunchWaveAlgorithm_rec(y, x, 0);
    }

    void LaunchWaveAlgorithm_rec(int y, int x, int curDepth)
    {
        if (!Pos{x, y}.isInsideMap())
            return;
        
        int& map_v_ref = (*WA_map)[y][x];
        int unitCurDepth = WA_bMy ? curDepth : curDepth + GInf.WA_ENEMY;
        if (map_v_ref == GInf.WA_INVALID || (map_v_ref <= unitCurDepth))
            return;
        
        // Found enemy
        if (WA_bMy && map_v_ref >= GInf.WA_ENEMY)
        {
            int enemyRealDepth = map_v_ref - GInf.WA_ENEMY;
            if (unitCurDepth >= enemyRealDepth)
                return;
        }
        map_v_ref = unitCurDepth;

        ++curDepth;
        if (curDepth == WA_depth)
            return;
        
        if (bOppRight)
        {
            LaunchWaveAlgorithm_rec(y,   x+1, curDepth);
            LaunchWaveAlgorithm_rec(y-1, x,   curDepth);
            LaunchWaveAlgorithm_rec(y+1, x,   curDepth);
            LaunchWaveAlgorithm_rec(y,   x-1, curDepth);
        }
        else
        {
            LaunchWaveAlgorithm_rec(y,   x-1, curDepth);
            LaunchWaveAlgorithm_rec(y-1, x,   curDepth);
            LaunchWaveAlgorithm_rec(y+1, x,   curDepth);
            LaunchWaveAlgorithm_rec(y,   x+1, curDepth);
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
        DBG_V(cur);
        DBG_V(target);
        std::swap(cur, target);

        std::vector<Pos> route{};
        int curDepth = map[cur.y][cur.x];
        DBG_V(curDepth);
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
                if (c->recycler)
                    continue;
                
                for (auto pNeighbour : c->getCellNeighbours())
                    if (pNeighbour->recycler)
                        continue;

                Pos p1 = bEnemyBottom ? c->getBottom() : c->getTop();
                Pos p2 = bOppRight    ? c->getRight()  : c->getLeft();

                // cell valid and not in my route
                if (GInf.isValid(p1) && std::count(route.begin(), route.end(), p1) == 0 ||
                    GInf.isValid(p2) && std::count(route.begin(), route.end(), p2) == 0)
                    cellsToProtect.emplace_back(route[i]);
            }

            int curLen = 0;
            for (int i = 0; i < cellsToProtect.size(); ++i)
            {
                auto p = cellsToProtect[i];
                if (curLen == 1 && p.distanceTo(cellsToProtect[i-1]) != 1) // new cell is far from previous. Add recycler
                {
                    recyclers.emplace_back(cellsToProtect[i-1]);
                    curLen = 0;
                }
                else if (curLen == 2)
                {
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

    Pos GetNextButterflyWingStep(std::vector<std::vector<int>>& map, Pos myP, Pos enemyP, int extYLim, bool bTopRobot)
    {
        // bottom
        //if (myP.isValid && getEnemyNeighboursCount_D1(&GInf.getCell(myP)) == 0)
        if (myP.isValid)
        {
            if ((bTopRobot && myP.y == 0 || !bTopRobot && myP.y == g_mapHeight-1)
                && getEnemyUnitNeighboursCount_D2(&GInf.getCell(myP)) != 0)
                return Pos{};

            DBG_V(myP);
            DBG_V(enemyP);

            for (auto pEnemyCell : GInf.enemyCUnits)
            {
                LaunchWaveAlgorithm(pEnemyCell->p.y, pEnemyCell->p.x, &map, 20, extYLim, false, bTopRobot ? Dir::TOP : Dir::BOT); // Enemy first estimation
            }
            LaunchWaveAlgorithm(myP.y,    myP.x,    &map, 20, extYLim, true,  bTopRobot ? Dir::TOP : Dir::BOT); // My estimation
            
            for (int y = 0; y < g_mapHeight; ++y)
            {
                std::cerr << std::endl;
                for (int x = 0; x < g_mapWidth; ++x)
                    std::cerr << ' ' << std::setw(3) << map[y][x];
            }
            std::cerr << std::endl;

            // Determine target
            DBG_V(GInf.bAsymmetricMap);
            Pos targetP{};
            
            auto isMyCell = [&map](int x, int y) { return map[y][x] != GInf.WA_INVALID && map[y][x] < GInf.WA_ENEMY; };
            if (!GInf.bAsymmetricMap)
            {
                std::vector<int> targetYToCheck{};
                targetYToCheck.resize(g_mapHeight);
                //if (bTopRobot) targetYToCheck = { 0, 1, 2, 3 };
                //else           targetYToCheck = { (int)map.size() - 1, (int)map.size() - 2 };
                if (bTopRobot) std::iota(targetYToCheck.begin(), targetYToCheck.end(), 0);
                else           std::iota(targetYToCheck.rbegin(), targetYToCheck.rend(), 0);

                if (bOppRight)
                {
                    for (int y : targetYToCheck)
                    {
                        int x = 0;
                        bool bFound = false;
                        for (; x < map[y].size() - 2; ++x)
                            if (isMyCell(x, y) && map[y][x+1] >= GInf.WA_ENEMY)
                            {
                                bFound = true;
                                break;
                            }

                        DBG_V2(x, y);
                        // Maybe no enemies?
                        if (!bFound)
                        {
                            for (x = map[y].size() - 1; x >= 0; --x)
                                if (map[y][x] != GInf.WA_INVALID)
                                {
                                    break;
                                }
                            if (isMyCell(x, y))
                                bFound = true;

                        }

                        if (bFound)
                        {
                            //if (y != 0 map[y][x])
                            //targetP = Pos{ x - 1, y, true };
                            targetP = Pos{ x, y, true };
                            break;
                        }
                    }
                }
                else
                {
                    for (int y : targetYToCheck)
                    {
                        int x = map[y].size() - 2;
                        bool bFound = false;
                        for (; x > 0; --x)
                            if (isMyCell(x, y) && map[y][x-1] >= GInf.WA_ENEMY)
                            {
                                bFound = true;
                                break;
                            }

                        // Maybe no enemies?
                        if (!bFound)
                        {
                            for (x = 0; x < map[y].size() ; ++x)
                                if (map[y][x] != GInf.WA_INVALID)
                                {
                                    break;
                                }
                            if (isMyCell(x, y))
                                bFound = true;

                        }

                        if (bFound)
                        {
                            //targetP = Pos{ x + 1, y, true };
                            targetP = Pos{ x, y, true };
                            break;
                        }
                    }
                }
            }
            else
            {
                std::vector<int> xToIterate{}, yToIterate{};
                xToIterate.resize((int)map[0].size()-1);
                yToIterate.resize((int)map.size());

                if (bOppRight) std::iota(std::begin(xToIterate),  std::end(xToIterate), 0);   // 0, 1, 2
                else           std::iota(std::rbegin(xToIterate), std::rend(xToIterate),  0); // 9, 8, 7
                
                if (bTopRobot) std::iota(std::begin(yToIterate), std::end(yToIterate), 0);     // 0, 1, 2
                else           std::iota(std::rbegin(yToIterate),  std::rend(yToIterate),  0); // 9, 8, 7

                DBG_MSG_ARR_V(xToIterate);
                DBG_MSG_ARR_V(yToIterate);
                for (int y : yToIterate)
                {
                    bool bFound = false;
                    for (int x : xToIterate)
                        {
                            if (map[y][x] > 0 && map[y][x] < GInf.WA_ENEMY && ((bOppRight ? map[y][x+1] : map[y][x-1]) >= GInf.WA_ENEMY))
                            {
                                targetP = Pos{ x, y, true };
                                bFound = true;
                                break;
                            }
                        }
                    
                    // Maybe no enemies?
                    if (!bFound)
                    {
                        auto itX = xToIterate.rbegin();
                        for (; itX != xToIterate.rend(); ++itX)
                                if (map[y][*itX] != GInf.WA_INVALID)
                                {
                                    targetP = Pos{ *itX, y, true };
                                    break;
                                }
                        if (isMyCell(*itX, y))
                            bFound = true;

                    }

                    if (bFound) break;
                }
            }


            DBG_MSG_V("Reseacher targets: ", targetP);
        
            if (!targetP.isValid || !isMyCell(targetP.x, targetP.y))
            {
                DBG_MSG_STR("!!!!!!!!!!!!!!! Failed to initialize Butterfly strategy. Can't calculate  reseacher targets. Terminate algorithm");
                return{};
            }

            // Robot tried to go to the previous cell in the route. Terminate algorithm
            for (auto p : (bTopRobot ? rTop_Route : rBot_Route))
                if (targetP == p)
                {
                    DBG_MSG_STR("!!!!!!!!!!!!!!! New butterfly wing target already in the route. Terminate algorithm");
                    return{};
                }

            // Build route to targets
            auto route = BuildRouteFromWave(map, myP, targetP, bOppRight ? Dir::RIGHT : Dir::LEFT);

            return !route.empty() ? route[0] : Pos{};
        }
        
        return {};
    }

    bool bOppRight = false; // if true - my units on the left, enemy on the right
    bool bNeedToProtectRoute = true;

    // Reseachers
    Pos rTop_Pos{ INT32_MAX, INT32_MAX };
    Pos rBot_Pos{ -1, -1 };
    Pos rTop_initPos{ INT32_MAX, INT32_MAX };
    Pos rBot_initPos{ -1, -1 };

    Pos rTop_NextStep{};
    Pos rBot_NextStep{};

    std::vector<Pos> rTop_Route{};
    std::vector<Pos> rBot_Route{};


    // Recyclers
    std::vector<Pos> recyclers;

    // Phase2 reseacher
    Pos researcherPos{};

    // Cell Destroyer
    Pos cellDestroyerPos{};
    Pos cellDestroyerTarget{};
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
