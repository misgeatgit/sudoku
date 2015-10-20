/* sudoku.cc 
 * Copyright (C) <2009-2016> <misgana.bayetta@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;

class Solver {
private:
    vector<int> _board;
    int _bsize;
    vector<int> _allowable_vals;

    struct box_candidates {
	int row, col;
	vector<int> cval;
    };
    using candidates = vector<box_candidates>;

    vector<int> disjoint(const vector<int>& val_existing);
    vector<int> intersect(const vector<vector<int> >& cell_candidates);

    vector<int> grid_candidate(const vector<int>& board, int bsize, int rstart, int cstart);
    vector<int> row_candidate(const vector<int>& board, int bsize, int rindex);
    vector<int> column_candidate(const vector<int>& board, int bsize, int colindex);

    candidates build_candidates(const vector<int>& board, int bsize);
    bool back_track(vector<int>& board, int bsize);

public:
    Solver(const vector<int>& board, int bsize);
    vector<int> solve();
};

Solver::Solver(const vector<int>& board, int bsize)
{
    _bsize = bsize;
    _board = board;
    for (int i = 1; i <= _bsize; i++)
	_allowable_vals.push_back(i);
};

vector<int> Solver::solve()
{
    bool soln = back_track(_board, _bsize);
    if (soln)
	cout << "Solved" << endl;
    else
	cout << "No soln" << endl;
    return _board;
}

Solver::candidates Solver::build_candidates(const vector<int>& board, int bsize)
{
    vector<vector<int> > grids;
    int step = sqrt(bsize);
    for (int rstart = 0; rstart <= bsize - step; rstart += step) {
	for (int cstart = 0; cstart <= bsize - step; cstart += step) {
	    grids.push_back(grid_candidate(board, bsize, rstart, cstart));
	}
    }

    auto print = [](vector<int>& v) { for(const auto& i:v) cout << i << " "; cout << endl; };
    candidates cdt;
    for (int i = 0; i < bsize; i++) {
	for (int j = 0; j < bsize; j++) {
	    if (board[bsize * i + j] == 0) {
		box_candidates bc;
		bc.row = i;
		bc.col = j;
		vector<int> gridcnd = grids[(i / step) * step + (j / step)]; //tricky hmm
		vector<int> colcnd = column_candidate(board, bsize, j);
		vector<int> rowcnd = row_candidate(board, bsize, i);
		bc.cval = intersect({ gridcnd, colcnd, rowcnd });
		cdt.push_back(bc);
	    }
	}
    }

    return cdt;
};

vector<int> Solver::grid_candidate(const vector<int>& board, int bsize, int rstart, int cstart)
{
    vector<int> existing;
    int grid_size = sqrt(bsize);

    for (int i = rstart; i < rstart + grid_size; i++) {
	for (int j = cstart; j < cstart + grid_size; j++) {
	    if (board[bsize * i + j] != 0) {
		existing.push_back(board[bsize * i + j]);
	    }
	}
    }
    return disjoint(existing);
}

vector<int> Solver::row_candidate(const vector<int>& board, int bsize, int rindex)
{
    vector<int> existing;
    for (int j = 0; j < bsize; j++) {
	if (board[bsize * rindex + j] != 0) {
	    existing.push_back(_board[bsize * rindex + j]);
	}
    }
    return disjoint(existing);
}

vector<int> Solver::column_candidate(const vector<int>& board, int bsize, int colindex)
{
    vector<int> existing;
    for (auto i = 0; i < bsize; i++) {
	if (board[bsize * i + colindex] != 0) {
	    existing.push_back(_board[bsize * i + colindex]);
	}
    }
    return disjoint(existing);
}

vector<int> Solver::intersect(const vector<vector<int> >& cell_candidates)
{
    vector<int> intersect;
    for (auto it = cell_candidates.begin(); it != cell_candidates.end(); ++it) {
	vector<int> temp;
	if (distance(cell_candidates.begin(), it) == 0) {
	    intersect = *it;
	}
	else {
	    sort(intersect.begin(), intersect.end());
	    auto cdt = *it;
	    sort(cdt.begin(), cdt.end());
	    set_intersection(intersect.begin(), intersect.end(), cdt.begin(),
		cdt.end(), back_inserter(temp));
	    intersect = temp;
	}
    }
    return intersect;
}

vector<int> Solver::disjoint(const vector<int>& val_existing)
{
    vector<int> disjoint_set;
    for (auto val : _allowable_vals) {
	if (find(val_existing.begin(), val_existing.end(), val) == val_existing.end())
	    disjoint_set.push_back(val);
    }

    return disjoint_set;
}

bool Solver::back_track(vector<int>& board, int bsize)
{

    auto done = [&board]() {
    for(auto& v : board){ if(v == 0 ) return false;}
    return true;
    };

    if (done())
	return true;

    auto insertable = [&board, bsize](int r, int c, int val) {
               for(int i =0; i< bsize; i++){
		       if(board[i*bsize+c] == val){return false;}
	       }
      	       for(int j =0; j < bsize; j++){
		       if(board[r*bsize+j] == val) {return false;}
	       }	

	       return true;
    };

    auto thinnest = [](const candidates& cdt) {
       int index;
       for(int i = 0; i< cdt.size(); i++){
	       if(i == 0) index = i;
	       else{
	       if(cdt[i].cval.size() < cdt[index].cval.size()) index = i; 
	       }
       }

       return index;
    };

    int val;
    bool works = false;

    auto cdt = build_candidates(board, bsize);
    //Lets start from the min candidate size.Good heuristics.
    box_candidates bc = cdt[thinnest(cdt)];
    while (not bc.cval.empty()) {
	val = bc.cval[0];
	bc.cval.erase(bc.cval.begin() + 0);

	if (insertable(bc.row, bc.col, val)) {
	    board[bsize * bc.row + bc.col] = val;
	    works = back_track(board, bsize);
	    if (not works) {
		board[bsize * bc.row + bc.col] = 0;
		continue;
	    }

	    else {
		break;
	    }
	}
    }

    return works;
}

int main(int argc, char** args)
{

    //Please use cat file | ./thisprogram. Untill file reading is implemented.
    //file should contain number of boards and then board size followed  size*size matrix
    // of the sudoku board for each board.
    //blank spaces should be denoted by 0.
    auto print_board = [](const vector<int>& board, int bsize) {
        for(int i = 0; i<bsize; i++){
		for(int j=0; j<bsize; j++){
			cout << board[bsize*i+j] << " ";
		}
		cout << "\n";
	}

    };

    int boards;
    cin >> boards;

    for (int i = 0; i < boards; i++) {
	int bsize;
	cin >> bsize;
	vector<int> board;
	board.resize(bsize * bsize);
	for (int i = 0; i < bsize; i++) {
	    for (int j = 0; j < bsize; j++) {
		cin >> board[i * bsize + j];
	    }
	}

	cout << "----------BOARD-----------------"
	     << "\n";
	print_board(board, bsize);
	Solver solver(board, bsize);
	vector<int> soln = solver.solve();
	cout << "----------SOLUTION--------------"
	     << "\n";
	print_board(soln, bsize);
    }

    /*vector<int> test = { 0, 0, 4, 0, 5, 3, 0, 0, 6,
	0, 0, 1, 4, 0, 2, 0, 0, 0,
	3, 0, 6, 8, 0, 0, 0, 0, 0,
	0, 9, 3, 0, 0, 0, 8, 0, 0,
	0, 2, 5, 3, 0, 1, 6, 7, 0,
	0, 0, 8, 0, 0, 0, 4, 2, 0,
	0, 0, 0, 0, 0, 5, 3, 0, 7,
	0, 0, 0, 7, 0, 9, 1, 0, 0,
	5, 0, 0, 1, 4, 0, 9, 0, 0 };

    cout << "----------BOARD-----------------"
	 << "\n";
    print_board(test, 9);
    Solver solver(test, 9);
    vector<int> soln = solver.solve();
    cout << "----------SOLUTION--------------"
	 << "\n";
    print_board(soln, 9);
         
    vector<int> tough = { 0, 0, 5, 3, 0, 0, 0, 0, 0,
	8, 0, 0, 0, 0, 0, 0, 2, 0,
	0, 7, 0, 0, 1, 0, 5, 0, 0,
	4, 0, 0, 0, 0, 5, 3, 0, 0,
	0, 1, 0, 0, 7, 0, 0, 0, 6,
	0, 0, 3, 2, 0, 0, 0, 8, 0,
	0, 6, 0, 5, 0, 0, 0, 0, 9,
	0, 0, 4, 0, 0, 0, 0, 3, 0,
	0, 0, 0, 0, 0, 9, 7, 0, 0 };
	
    cout << "----------BOARD-----------------"
	 << "\n";
    print_board(tough, 9);
    Solver solver(tough, 9);
    vector<int> soln = solver.solve();
    cout << "----------SOLUTION--------------"
	 << "\n";
    print_board(soln, 9);
*/
    return 0;
}
