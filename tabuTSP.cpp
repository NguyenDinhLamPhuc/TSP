#include <bits/stdc++.h>
#include <random>
using namespace std;

// -------------------- HÀM TÍNH COST --------------------
double tour_cost(const vector<int> &tour, const vector<vector<double>> &dist) {
    int n = tour.size();
    double s = 0;
    for (int i = 0; i < n; i++) {
        s += dist[tour[i]][tour[(i + 1) % n]];
    }
    return s;
}

// -------------------- KHỞI TẠO GREEDY --------------------
vector<int> nearest_neighbor_init(int n, const vector<vector<double>> &dist, int start = 0) {
    vector<int> tour;
    vector<bool> used(n, false);
    int cur = start;
    tour.push_back(cur);
    used[cur] = true;

    for (int step = 1; step < n; step++) {
        int best = -1;
        double bestd = 1e18;
        for (int j = 0; j < n; j++) {
            if (!used[j] && dist[cur][j] < bestd) {
                bestd = dist[cur][j];
                best = j;
            }
        }
        tour.push_back(best);
        used[best] = true;
        cur = best;
    }
    return tour;
}

// -------------------- PHÉP BIẾN ĐỔI: DI CHUYỂN 2 THÀNH PHỐ --------------------
// Di chuyển cặp (i, i+1) sang vị trí k (k là index trong *mảng gốc* trước khi xóa).
// Trong hàm ta tính insert_pos trong mảng đã xóa 2 phần tử:
//  - nếu k <= i: insert_pos = k
//  - nếu k > i+1: insert_pos = k - 2
// Chú ý: không cho phép k == i hoặc k == i+1 (chèn trùng/overlap).
void move_two_cities(vector<int> &tour, int i, int k) {
    int n = tour.size();
    if (i < 0 || i + 1 >= n) return; // i phải có i+1
    if (k < 0 || k >= n) return; // k là index trong mảng ban đầu

    // cấm chèn vào vùng overlap của chính nó
    if (k == i || k == i + 1 || k == i-1) return;

    int city1 = tour[i];
    int city2 = tour[i + 1];

    // remove two elements at i and i+1
    tour.erase(tour.begin() + i, tour.begin() + i + 2); // size -> n-2

    int insert_pos;
    if (k < i) insert_pos = k+1;
    else insert_pos = k - 1; // đã xóa 2 phần tử trước vị trí k

    vector<int> tmp = {city1, city2};
    tour.insert(tour.begin() + insert_pos, tmp.begin(), tmp.end());
}

// -------------------- PHÉP BIẾN ĐỔI: ĐỔI CHỖ 2 THÀNH PHỐ --------------------
void swap_two_cities(vector<int> &tour, int i, int j) {
    int n = tour.size();
    if (i < 0 || i >= n || j < 0 || j >= n || i == j) return;
    swap(tour[i], tour[j]);
}

// -------------------- ĐỌC FILE .TSP --------------------
// (Giữ nguyên hoàn toàn hàm read_tsp từ code gốc của bạn.)
void read_tsp(const string &filename, vector<vector<double>> &dist, int &n) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        cerr << "Không mở được file " << filename << endl;
        exit(1);
    }

    vector<string> lines;
    string line;
    while (getline(fin, line)) lines.push_back(line);
    fin.close();

    auto up = [&](string s){ transform(s.begin(), s.end(), s.begin(), ::toupper); return s; };
    auto trim = [&](string s){
        auto a = s.find_first_not_of(" \t\r\n");
        if (a==string::npos) return string();
        auto b = s.find_last_not_of(" \t\r\n");
        return s.substr(a, b-a+1);
    };

    string edge_weight_type = "EXPLICIT";
    string edge_weight_format = "";
    string display_data_type = "";
    int dimension = -1;

    int L = lines.size();
    int edge_section_line = -1, nodecoord_line = -1, display_line = -1;
    for (int i = 0; i < L; ++i) {
        string s = up(lines[i]);
        if (s.find("DIMENSION") != string::npos) {
            auto pos = lines[i].find(':');
            if (pos != string::npos) dimension = stoi(trim(lines[i].substr(pos+1)));
            else {
                stringstream ss(lines[i]);
                string tok; ss >> tok; ss >> tok;
                dimension = stoi(tok);
            }
        }
        else if (s.find("EDGE_WEIGHT_TYPE") != string::npos) {
            auto pos = lines[i].find(':');
            string val = (pos==string::npos? lines[i] : lines[i].substr(pos+1));
            edge_weight_type = up(trim(val));
        }
        else if (s.find("EDGE_WEIGHT_FORMAT") != string::npos) {
            auto pos = lines[i].find(':');
            string val = (pos==string::npos? lines[i] : lines[i].substr(pos+1));
            edge_weight_format = up(trim(val));
        }
        else if (s.find("DISPLAY_DATA_TYPE") != string::npos) {
            auto pos = lines[i].find(':');
            string val = (pos==string::npos? lines[i] : lines[i].substr(pos+1));
            display_data_type = up(trim(val));
        }
        else if (s.find("EDGE_WEIGHT_SECTION") != string::npos) edge_section_line = i;
        else if (s.find("NODE_COORD_SECTION") != string::npos) nodecoord_line = i;
        else if (s.find("DISPLAY_DATA_SECTION") != string::npos) display_line = i;
    }

    if (dimension <= 0) {
        cerr << "Không xác định DIMENSION trong file\n";
        exit(1);
    }
    n = dimension;
    dist.assign(n, vector<double>(n, 0.0));

    auto collect_numbers_from = [&](int start_line, long needed) {
        vector<double> vals;
        for (int i = start_line; i < L && (long)vals.size() < needed; ++i) {
            stringstream ss(lines[i]);
            double v;
            while (ss >> v) {
                vals.push_back(v);
                if ((long)vals.size() >= needed) break;
            }
        }
        return vals;
    };

    if ((edge_weight_type.find("EUC_2D") != string::npos || edge_weight_type.find("EUC2D") != string::npos)
        && (nodecoord_line != -1 || display_line != -1)) {
        int start = (nodecoord_line != -1 ? nodecoord_line + 1 : display_line + 1);
        vector<pair<double,double>> coord(n, {0,0});
        int idx = 0;
        for (int i = start; i < L && idx < n; ++i) {
            string s = trim(lines[i]);
            if (s.empty()) continue;
            stringstream ss(s);
            int id;
            double x, y;
            if (!(ss >> id >> x >> y)) {
                vector<double> toks;
                double t;
                stringstream ss2(s);
                while (ss2 >> t) toks.push_back(t);
                if (toks.size() >= 2) {
                    x = toks[toks.size()-2];
                    y = toks[toks.size()-1];
                    id = idx+1;
                } else continue;
            }
            if (id >= 1 && id <= n) coord[id-1] = {x,y};
            else coord[idx] = {x,y};
            idx++;
        }
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i==j) { dist[i][j] = 0; continue; }
                double dx = coord[i].first - coord[j].first;
                double dy = coord[i].second - coord[j].second;
                double d = sqrt(dx*dx + dy*dy);
                dist[i][j] = floor(d + 0.5);
            }
        }
        return;
    }

    if (edge_weight_type.find("GEO") != string::npos && nodecoord_line != -1) {
        int start = nodecoord_line + 1;
        vector<pair<double,double>> coord(n, {0,0});
        int idx = 0;
        for (int i = start; i < L && idx < n; ++i) {
            string s = trim(lines[i]);
            if (s.empty()) continue;
            stringstream ss(s);
            int id; double x, y;
            if (!(ss >> id >> x >> y)) {
                vector<double> toks; double t; stringstream ss2(s);
                while (ss2 >> t) toks.push_back(t);
                if (toks.size() >= 2) { x = toks[toks.size()-2]; y = toks[toks.size()-1]; id = idx+1; }
                else continue;
            }
            if (id >= 1 && id <= n) coord[id-1] = {x,y};
            else coord[idx] = {x,y};
            idx++;
        }
        auto to_rad_geo = [&](double v)->double{
            int deg = (int)floor(v);
            double min = v - deg;
            return M_PI * (deg + 5.0 * min / 3.0) / 180.0;
        };
        vector<double> lat(n), lon(n);
        for (int i = 0; i < n; ++i) {
            lat[i] = to_rad_geo(coord[i].first);
            lon[i] = to_rad_geo(coord[i].second);
        }
        const double R = 6378.388;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i==j) { dist[i][j]=0; continue; }
                double q1 = cos(lon[i]-lon[j]);
                double dij = R * acos( sin(lat[i])*sin(lat[j]) + cos(lat[i])*cos(lat[j])*q1 );
                dist[i][j] = floor(dij + 1.0);
            }
        }
        return;
    }

    if (edge_section_line != -1) {
        string fmt = edge_weight_format;
        if (fmt.empty()) fmt = "UPPER_ROW";
        long need = 0;
        if (fmt.find("FULL") != string::npos) need = 1L * n * n;
        else if (fmt.find("LOWER_DIAG") != string::npos || fmt.find("LOWERDIAG") != string::npos) need = 1L * n * (n+1) / 2;
        else if (fmt.find("UPPER_DIAG") != string::npos || fmt.find("UPPERDIAG") != string::npos) need = 1L * n * (n+1) / 2;
        else if (fmt.find("LOWER_ROW") != string::npos || (fmt.find("LOWER")!=string::npos && fmt.find("ROW")!=string::npos)) need = 1L * n * (n-1) / 2;
        else if (fmt.find("UPPER_ROW") != string::npos || (fmt.find("UPPER")!=string::npos && fmt.find("ROW")!=string::npos)) need = 1L * n * (n-1) / 2;
        else need = LONG_MAX;

        vector<double> values;
        for (int i = edge_section_line + 1; i < L; ++i) {
            string U = up(lines[i]);
            if (U.find("DISPLAY_DATA_SECTION")!=string::npos || U.find("NODE_COORD_SECTION")!=string::npos
                || U.find("TOUR_SECTION")!=string::npos || U.find("EOF")!=string::npos) break;
            stringstream ss(lines[i]);
            double v;
            while (ss >> v) {
                values.push_back(v);
                if ((long)values.size() == need) break;
            }
            if ((long)values.size() == need) break;
        }

        if (need != LONG_MAX && (long)values.size() < need) {
            cerr << "Dữ liệu EDGE_WEIGHT_SECTION không đủ: cần " << need << " giá trị, có " << values.size() << "\n";
            exit(1);
        }

        long idx = 0;
        if (fmt.find("FULL") != string::npos) {
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    dist[i][j] = values[idx++];
        }
        else if (fmt.find("UPPER_ROW") != string::npos) {
            for (int i = 0; i < n-1; ++i)
                for (int j = i+1; j < n; ++j) {
                    dist[i][j] = values[idx];
                    dist[j][i] = values[idx];
                    idx++;
                }
        }
        else if (fmt.find("LOWER_ROW") != string::npos) {
            for (int j = 0; j < n-1; ++j)
                for (int i = j+1; i < n; ++i) {
                    dist[i][j] = values[idx];
                    dist[j][i] = values[idx];
                    idx++;
                }
        }
        else if (fmt.find("UPPER_DIAG") != string::npos) {
            for (int i = 0; i < n; ++i)
                for (int j = i; j < n; ++j) {
                    dist[i][j] = values[idx];
                    dist[j][i] = values[idx];
                    idx++;
                }
        }
        else if (fmt.find("LOWER_DIAG") != string::npos) {
            for (int i = 0; i < n; ++i)
                for (int j = 0; j <= i; ++j) {
                    dist[i][j] = values[idx];
                    dist[j][i] = values[idx];
                    idx++;
                }
        }
        else {
            idx = 0;
            for (int i = 0; i < n-1; ++i)
                for (int j = i+1; j < n; ++j) {
                    if (idx >= (int)values.size()) break;
                    dist[i][j] = values[idx];
                    dist[j][i] = values[idx];
                    idx++;
                }
        }
        return;
    }

    if (display_line != -1) {
        int start = display_line + 1;
        vector<pair<double,double>> coord(n, {0,0});
        int idx = 0;
        for (int i = start; i < L && idx < n; ++i) {
            string s = trim(lines[i]);
            if (s.empty()) continue;
            stringstream ss(s);
            int id; double x,y;
            if (!(ss >> id >> x >> y)) {
                vector<double> toks; double t; stringstream ss2(s);
                while (ss2 >> t) toks.push_back(t);
                if (toks.size() >= 2) { x = toks[toks.size()-2]; y = toks[toks.size()-1]; id = idx+1; }
                else continue;
            }
            if (id>=1 && id<=n) coord[id-1] = {x,y};
            else coord[idx] = {x,y};
            idx++;
        }
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i==j) { dist[i][j]=0; continue; }
                double dx = coord[i].first - coord[j].first;
                double dy = coord[i].second - coord[j].second;
                dist[i][j] = floor(sqrt(dx*dx + dy*dy) + 0.5);
            }
        }
        return;
    }

    cerr << "Không tìm thấy dữ liệu tọa độ hoặc EDGE_WEIGHT_SECTION phù hợp trong file\n";
    exit(1);
}

// -------------------- HÀM CHÍNH --------------------
int main(int argc, char* argv[]) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist01(0, 1); // 0 hoặc 1


    if (argc < 2) {
        cerr << "Usage: " << (argc > 0 ? argv[0] : "tabuTSP") << " <file.tsp>\n";
        return 1;
    }
    
    int n;
    vector<vector<double>> dist;

    string filename = argv[1];
    
    cout << "Testing: " << filename << endl;
    read_tsp(filename, dist, n);
    
    // ---- THAM SỐ TABU ----
    const int MAX_ITER = 20000;
    const int TABU_TENURE = max(10, n / 5); // số vòng tabu
    const int MAX_NO_IMPROVE = 4000;

    // ---- KHỞI TẠO GREEDY ----
    /*vector<int> curTour = nearest_neighbor_init(n, dist);
    double curCost = tour_cost(curTour, dist);
    vector<int> bestTour = curTour;
    double bestCost = curCost;*/

    vector<int> curTour(n);
    curTour[0] = 0; 
    for(int i = 1; i < n; i++) curTour[i] = i;
    shuffle(curTour.begin() + 1, curTour.end(), gen);
    double curCost = tour_cost(curTour, dist);
    vector<int> bestTour = curTour;
    double bestCost = curCost;

    // ---- DANH SÁCH TABU (lưu theo city id) ----
    vector<vector<int>> tabu_pair(n, vector<int>(n, 0));
    vector<vector<int>> tabu_swap(n, vector<int>(n, 0));
    vector<int> tabu_move(n, 0); // tabu cho move

    int iter = 0, noImprove = 0;

    while (iter < MAX_ITER && noImprove < MAX_NO_IMPROVE) {
        iter++;
        double bestNeighborCost = 1e18;
        vector<int> bestNeighborTour;
        int bestA = -1, bestB = -1, bestC = -1; // city ids for tabu bookkeeping
        enum MoveType { NONE, SWAP, MOVE_PAIR } bestType = NONE;
        bool useSwap = dist01(gen);
        if (useSwap) {
            for (int i = 1; i < n; ++i) {
                for (int k = i+1; k < n; ++k) { 
                    vector<int> cand = curTour;
                    swap_two_cities(cand, i, k);
                    double candCost = tour_cost(cand, dist);

                    int a = curTour[i];
                    int b = curTour[k];
                    bool isTabu = (tabu_swap[a][b] > 0) || (tabu_swap[b][a] > 0);

                    // aspiration: nếu cải thiện global best thì chấp nhận dù tabu
                    if (isTabu && !(candCost < bestCost)) continue;

                    if (candCost < bestNeighborCost) {
                        bestNeighborCost = candCost;
                        bestNeighborTour = cand;
                        bestA = a; bestB = b;
                        bestType = SWAP;
                    }
                }
            }
        } else { // --- đánh giá move_pair neighbors ---
            for (int i = 1; i <= n-2; ++i) {
                for (int k = 0; k < n; ++k) {
                    if (k == i || k == i+1 || k == i-1) continue; // skip overlapping insert positions
                    vector<int> cand = curTour;
                    move_two_cities(cand, i, k);
                    double candCost = tour_cost(cand, dist);

                    int a = curTour[i];
                    int b = curTour[i+1];
                    int c = curTour[k];
                    bool isTabu = (tabu_pair[a][b] > 0) || (tabu_move[c] > 0);

                    if (isTabu && !(candCost < bestCost)) continue;

                    if (candCost < bestNeighborCost) {
                        bestNeighborCost = candCost;
                        bestNeighborTour = cand;
                        bestA = a; bestB = b; bestC = c;
                        bestType = MOVE_PAIR;
                    }
                }
            }
        }

        // nếu không tìm thấy neighbor tốt nào thì giảm tabu và dừng
        if (bestType == NONE || bestNeighborTour.empty()) {
            for (int x = 0; x < n; x++){
                if (tabu_move[x] > 0) tabu_move[x]--;
                for (int y = 0; y < n; y++){
                    if (tabu_pair[x][y] > 0) tabu_pair[x][y]--;
                    if (tabu_swap[x][y] > 0) tabu_swap[x][y]--;
                }
            }    
            break;
        }

        // apply chosen neighbor
        curTour = bestNeighborTour;
        curCost = bestNeighborCost;

        // update tabu list on city ids
        if (bestType == SWAP) {
            tabu_swap[bestA][bestB] = TABU_TENURE;
            tabu_swap[bestB][bestA] = TABU_TENURE;
        } else if (bestType == MOVE_PAIR) {
            tabu_pair[bestA][bestB] = TABU_TENURE;
            tabu_move[bestC] = TABU_TENURE;
        }

        // update global best
        if (curCost < bestCost) {
            bestCost = curCost;
            bestTour = curTour;
            noImprove = 0;
        } else noImprove++;

        // decrement tabu tenures
        for (int x = 0; x < n; x++){
            if (tabu_move[x] > 0) tabu_move[x]--;
            for (int y = 0; y < n; y++){
                if (tabu_pair[x][y] > 0) tabu_pair[x][y]--;
                if (tabu_swap[x][y] > 0) tabu_swap[x][y]--;
            }
        }


        // nhỏ: in tiến trình mỗi 100 vòng để người dùng thấy chương trình đang chạy
        if (iter % 1000 == 0) {
            cerr << "Iter=" << iter << " bestCost=" << bestCost << " noImprove=" << noImprove << "\n";
        }
    }

    cout << fixed << setprecision(6);
    cout << "\nBest cost: " << bestCost << "\nBest tour:\n";
    for (int city : bestTour) cout << city << ' ';
    cout << "0\n";
    return 0;
}

