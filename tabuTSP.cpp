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

    string line;
    bool found_edge_section = false;
    string edge_format = "UPPER_ROW"; // mặc định cũ nếu không khai báo

    // Đọc các thông tin header
    while (getline(fin, line)) {
        string up = line;
        // chuyển về chữ hoa để so sánh không phân biệt hoa thường
        transform(up.begin(), up.end(), up.begin(), ::toupper);

        if (up.find("DIMENSION") != string::npos) {
            auto pos = line.find(":");
            if (pos != string::npos) {
                string tmp = line.substr(pos + 1);
                n = stoi(tmp);
            } else {
                // nếu không có ":", có thể token thứ 2 là số
                stringstream ss(line);
                string token;
                ss >> token; // DIMENSION
                ss >> token; // số
                n = stoi(token);
            }
        } else if (up.find("EDGE_WEIGHT_FORMAT") != string::npos) {
            auto pos = line.find(":");
            string val;
            if (pos != string::npos) val = line.substr(pos + 1);
            else {
                stringstream ss(line);
                string t;
                ss >> t; // EDGE_WEIGHT_FORMAT
                ss >> t; // value
                val = t;
            }
            // trim
            auto start = val.find_first_not_of(" \t");
            auto end = val.find_last_not_of(" \t");
            if (start != string::npos) val = val.substr(start, end - start + 1);
            // chuyển về chữ hoa
            transform(val.begin(), val.end(), val.begin(), ::toupper);
            edge_format = val;
        } else if (up.find("EDGE_WEIGHT_SECTION") != string::npos) {
            found_edge_section = true;
            break;
        }
    }

    if (!found_edge_section) {
        cerr << "Không tìm thấy EDGE_WEIGHT_SECTION trong file!\n";
        exit(1);
    }

    dist.assign(n, vector<double>(n, 0));
    vector<double> values;
    double val;
    while (fin >> val) {
        values.push_back(val);
    }

    int idx = 0;
    if (edge_format == "LOWER_DIAG_ROW" || edge_format == "LOWER_DIAG_ROW.") {
        // đọc tam giác dưới có đường chéo: hàng i chứa j = 0..i
        for (int i = 0; i < n; i++) {
            for (int j = 0; j <= i; j++) {
                if (idx >= (int)values.size()) {
                    cerr << "Dữ liệu không đủ cho LOWER_DIAG_ROW\n";
                    exit(1);
                }
                dist[i][j] = values[idx++];
                dist[j][i] = dist[i][j];
            }
        }
    } else if (edge_format == "UPPER_ROW" || edge_format == "UPPER_ROW.") {
        // như cũ: tam giác trên (không gồm đường chéo)
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                if (idx >= (int)values.size()) {
                    cerr << "Dữ liệu không đủ cho UPPER_ROW\n";
                    exit(1);
                }
                dist[i][j] = values[idx];
                dist[j][i] = values[idx];
                idx++;
            }
        }
    } else if (edge_format == "FULL_MATRIX" || edge_format == "FULL_MATRIX.") {
        // ma trận đầy đủ n*n
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (idx >= (int)values.size()) {
                    cerr << "Dữ liệu không đủ cho FULL_MATRIX\n";
                    exit(1);
                }
                dist[i][j] = values[idx++];
            }
        }
    } else {
        cerr << "EDGE_WEIGHT_FORMAT '" << edge_format << "' chưa được hỗ trợ.\n";
        exit(1);
    }
}

// -------------------- HÀM CHÍNH --------------------
int main() {
    int n;
    vector<vector<double>> dist;

    string filename = "bayg29.tsp";
    read_tsp(filename, dist, n);

    // ---- THAM SỐ TABU ----
    const int MAX_ITER = 2000;
    const int TABU_TENURE = 5;
    const int MAX_NO_IMPROVE = 400;

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
                    swap_two_cities(cand, i, k);
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
