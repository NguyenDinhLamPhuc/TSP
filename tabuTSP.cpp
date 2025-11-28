#include <bits/stdc++.h>
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
void move_two_cities(vector<int> &tour, int i, int k) {

    if (k <= i && (i - k) <= 1) return;
    if (k >= i && (k - i) <= 2) return;
    int n = tour.size();
    int city1 = tour[i];
    int city2 = tour[i + 1];
    tour.erase(tour.begin() + i, tour.begin() + i + 2);  // cắt 2 thành phố
    if (k > i + 1) k -= 2; // điều chỉnh vị trí chèn sau khi xóa
    tour.insert(tour.begin() + k + 1, {city1, city2});  // chèn lại
}

// -------------------- PHÉP BIẾN ĐỔI: ĐỔI CHỖ 2 THÀNH PHỐ --------------------
void swap_two_cities(vector<int> &tour, int i, int j) {
    int n = tour.size();
    if (i < 0 || i >= n || j < 0 || j >= n || i == j) return;
    swap(tour[i], tour[j]);
}

// -------------------- ĐỌC FILE .TSP --------------------
void read_tsp(const string &filename, vector<vector<double>> &dist, int &n) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        cerr << "Không mở được file " << filename << endl;
        exit(1);
    }

    // đọc toàn bộ file vào vector dòng để xử lý linh hoạt các section
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

    // header fields mặc định
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

    // helper: parse tokens (số) từ dòng d tới d + nhiều
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

    // nếu có NODE_COORD_SECTION hoặc DISPLAY_DATA_SECTION và EDGE_WEIGHT_TYPE == EUC_2D (hoặc kiểu tọa độ khác)
    if ((edge_weight_type.find("EUC_2D") != string::npos || edge_weight_type.find("EUC2D") != string::npos)
        && (nodecoord_line != -1 || display_line != -1)) {
        // lấy tọa độ từ NODE_COORD_SECTION ưu tiên, nếu không có dùng DISPLAY_DATA_SECTION
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
                // một số file dùng định dạng "index: x y" hoặc "index x y" - cố gắng bắt số cuối hai số
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
        // tính khoảng cách Euclid (làm tròn theo TSPLIB)
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

    // hỗ trợ GEO nếu có NODE_COORD_SECTION
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
            // theo định nghĩa TSPLIB: chuyển minutes với factor 5/3 trước khi đổi sang rad
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

    // nếu có EDGE_WEIGHT_SECTION (EXPLICIT hoặc các format ma trận tam giác)
    if (edge_section_line != -1) {
        // chuẩn hóa tên format
        string fmt = edge_weight_format;
        if (fmt.empty()) {
            // một số file không khai báo; cố gắng đoán: nếu dòng sau có n*(n-1)/2 số -> UPPER_ROW/LOWER_ROW
            // nhưng ở đây sẽ ưu tiên UPPER_ROW nếu không biết
            fmt = "UPPER_ROW";
        }
        long need = 0;
        if (fmt.find("FULL") != string::npos) need = 1L * n * n;
        else if (fmt.find("LOWER_DIAG") != string::npos || fmt.find("LOWERDIAG") != string::npos) need = 1L * n * (n+1) / 2;
        else if (fmt.find("UPPER_DIAG") != string::npos || fmt.find("UPPERDIAG") != string::npos) need = 1L * n * (n+1) / 2;
        else if (fmt.find("LOWER_ROW") != string::npos || (fmt.find("LOWER")!=string::npos && fmt.find("ROW")!=string::npos)) need = 1L * n * (n-1) / 2;
        else if (fmt.find("UPPER_ROW") != string::npos || (fmt.find("UPPER")!=string::npos && fmt.find("ROW")!=string::npos)) need = 1L * n * (n-1) / 2;
        else {
            // fallback: nếu EXPLICIT và không biết format, đọc đến hết file (an toàn nếu EDGE_SECTION là cuối file)
            need = LONG_MAX;
        }

        vector<double> values;
        // thu thập số từ dòng tiếp theo của EDGE_WEIGHT_SECTION
        for (int i = edge_section_line + 1; i < L; ++i) {
            // nếu gặp 1 section khác thì dừng
            string U = up(lines[i]);
            if (U.find("DISPLAY_DATA_SECTION")!=string::npos || U.find("NODE_COORD_SECTION")!=string::npos
                || U.find("TOUR_SECTION")!=string::npos || U.find("EOF")!=string::npos) break;
            stringstream ss(lines[i]);
            double v;
            while (ss >> v) {
                values.push_back(v);
                if ((long)values.size() == need) break;
                // nếu need == LONG_MAX thì tiếp tục cho tới hết
            }
            if ((long)values.size() == need) break;
        }

        if (need != LONG_MAX && (long)values.size() < need) {
            cerr << "Dữ liệu EDGE_WEIGHT_SECTION không đủ: cần " << need << " giá trị, có " << values.size() << "\n";
            exit(1);
        }

        // điền vào ma trận dựa theo format
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
            // fallback: nếu không xác định, cố gắng điền theo UPPER_ROW
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

    // Nếu không có các section trên, thử đọc các cặp tọa độ trong file (thường là DISPLAY_DATA_SECTION cuối file)
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
    const int MAX_ITER = 5000;
    const int TABU_TENURE = 10;
    const int MAX_NO_IMPROVE = 1000;

    // ---- KHỞI TẠO GREEDY ----
    vector<int> curTour = nearest_neighbor_init(n, dist);
    double curCost = tour_cost(curTour, dist);
    vector<int> bestTour = curTour;
    double bestCost = curCost;
    // ---- DANH SÁCH TABU ----
    vector<vector<int>> tabu2_0(n, vector<int>(n, 0));
    vector<vector<int>> tabu1_1(n, vector<int>(n, 0));
    int iter = 0, noImprove = 0;
    bool useSwap = false; // Chuyển đổi giữa 2 phép biến đổi

    while (iter < MAX_ITER && noImprove < MAX_NO_IMPROVE) {
        iter++;
        double bestNeighborCost = 1e18;
        int bestI = -1, bestK = -1, bak = -1;
        vector<int> bestNeighborTour;

        for (int i = 1; i < n; i++) {
            for (int k = 1; k < n; k++) {
                vector<int> cand = curTour;
                bool isTabu = false;
                int a, b;
                if(useSwap){
                    move_two_cities(cand, i, k);
                    a = curTour[i];
                    b = curTour[i + 1];
                    isTabu = (tabu2_0[a][b] > 0) || (tabu2_0[b][a] > 0);
                    useSwap = false;
                }
                else {
                    swap_two_cities(cand, i, k);
                    a = curTour[i];
                    b = curTour[k];
                    isTabu = (tabu1_1[a][b] > 0) || (tabu1_1[b][a] > 0);
                    useSwap = true;
                }

                double candCost = tour_cost(cand, dist);
                bool improve = false;


                if (candCost < bestCost && candCost < bestNeighborCost) {
                    bestNeighborCost = candCost;
                    bestI = curTour[a];
                    bestK = curTour[b];
                    bestNeighborTour = cand;
                    improve = true;
                } else if (!improve) {
                    if (!isTabu && candCost < bestNeighborCost) {
                        bestNeighborCost = candCost;
                        bestI = curTour[a];
                        bestK = curTour[b];
                        bestNeighborTour = cand;
                    }
                }
            }
        }

        if (bestI == -1) {
            for (int x = 0; x < n; x++)
                for (int y = 0; y < n; y++){
                    if (tabu2_0[x][y] > 0) tabu2_0[x][y]--;
                    if (tabu1_1[x][y] > 0) tabu1_1[x][y]--;
                }
            break;
        }

        curTour = bestNeighborTour;
        curCost = bestNeighborCost;
        if(useSwap){
            tabu1_1[bestI][bestK] = TABU_TENURE;
            tabu1_1[bestK][bestI] = TABU_TENURE;
        }
        else{
            tabu2_0[bestI][bestK] = TABU_TENURE;
            tabu2_0[bestK][bestI] = TABU_TENURE;
        }

        if (curCost < bestCost) {
            bestCost = curCost;
            bestTour = curTour;
            noImprove = 0;
        } else noImprove++;

        for (int x = 0; x < n; x++)
            for (int y = 0; y < n; y++){
                if (tabu2_0[x][y] > 0) tabu2_0[x][y]--;
                if (tabu1_1[x][y] > 0) tabu1_1[x][y]--;
            }
        
    }

    cout << fixed << setprecision(6);
    cout << "\nBest cost: " << bestCost << "\nBest tour:\n";
    for (int city : bestTour) cout << city << ' ';
    cout << "0\n";
}
