#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
using namespace std;

struct Time {
	int h, m;

	Time() { h = m = 0;  }
	Time(int _h, int _m) : h(_h), m(_m) {}
	bool operator < (const Time &o) const {
		if (h != o.h) return h < o.h;
		if (m != o.m) return m < o.m;
		return false;
	}
	bool operator > (const Time &o) const {
		if (h != o.h) return h > o.h;
		if (m != o.m) return m > o.m;
		return false;
	}
	bool operator == (const Time &o) const {
		if (h != o.h) return false;
		if (m != o.m) return false;
		return true;
	}
	void read_time() {
		string tmp;
		cin >> tmp;
		h = (tmp[0] - '0') * 10 + tmp[1] - '0';
		m = (tmp[3] - '0') * 10 + tmp[4] - '0';
	}
	string print_time() {
		string ret = "";
		if (h <= 9) {
			ret.push_back('0');
			ret.push_back(h + '0');
		}
		else {
			ret.push_back(h / 10 + '0');
			ret.push_back(h % 10 + '0');
		}
		ret.push_back(':');
		if (m <= 9) {
			ret.push_back('0');
			ret.push_back(m + '0');
		}
		else {
			ret.push_back(m / 10 + '0');
			ret.push_back(m % 10 + '0');
		}
		return ret;
	}
	int return_minuts() {
		return h * 60 + m;
	}
};

struct Line {
	Time time;
	vector<string> line;
	Line() : time(Time(0, 0)) { }

	void read_line(Time tmp_time, string tmp_line) {
		time = tmp_time;
		tmp_line.push_back(' ');
		string tmp;
		for (auto i : tmp_line) {
			if (i != ' ')
				tmp.push_back(i);
			else {
				if (tmp != " ")
					line.push_back(tmp);
				tmp.clear();
			}
		}
	}
	void print_line() {
		cout << time.print_time();
		for (auto i : line) {
			cout << " " << i;
		}
		cout << endl;
	}
};

string f_serverlog, f_loot;
ifstream i_serverlog, i_loot;

Time start_time, end_time;
map<string, int> potions, loot;
int vampiryzm, health_losed, full_damage, health_gained_by_potions, health_gained_by_spells, exp_gained;


bool input_files() {
	cout << "Name of serverlog file: ";
	cin >> f_serverlog;
	cout << "Name of loot file: ";
	cin >> f_loot;
	f_serverlog = "serverlogs/" + f_serverlog;
	f_loot = "loots/" + f_loot;
	i_serverlog.open(f_serverlog.c_str());
	i_loot.open(f_loot.c_str());

	if (i_serverlog.is_open() && i_loot.is_open())
		return true;
	return false;
}

void input_times() {
	cout << "Begin time(hh:mm | 09:13 | 22:03): ";
	start_time.read_time();
	cout << "End time(hh:mm | 09:13 | 22:03): ";
	end_time.read_time();
}

#define UNDEFINED 0
#define SHOT_BY_PLAYER 1
#define SHOT_BY_MONSTER 2
#define VAMPIRYZM 3
#define HEAL_SPELL_POTION 4
#define USING_POTION 6
#define EXP_GAINED 7
#define LOOT 8

int check(Line &l) {
	if (l.line[0] == "A" || l.line[0] == "An")
		return SHOT_BY_PLAYER;
	if (l.line[0] == "You" && l.line[1] == "lose")
		return SHOT_BY_MONSTER;
	if (l.line[0] == "You" && l.line[1] == "were")
		return VAMPIRYZM;
	if (l.line[0] == "You" && l.line[1] == "healed" && l.line[2] == "yourself")
		return HEAL_SPELL_POTION;
	if (l.line[0] == "Using" && (l.line.back() == "potions." || l.line.back() == "potion."))
		return USING_POTION;
	if (l.line[0] == "You" && l.line[1] == "gained")
		return EXP_GAINED;
	if (l.line[0] == "Loot")
		return LOOT;
	return UNDEFINED;
}

int liczba(string x) {
	int ret = 0;
	bool ok = true;
	for (auto i : x) {
		if (i < '0' || i > '9') {
			ret = -1;
			break;
		}
		ret *= 10;
		ret += i - '0';
	}
	return ret;
}

void add_player_damage(Line &l) {
	for (auto i : l.line) {
		if (liczba(i) > 0) {
			full_damage += liczba(i);
			break;
		}
	}
}

void add_monster_damage(Line &l) {
	for (auto i : l.line) {
		if (liczba(i) > 0) {
			health_losed += liczba(i);
			break;
		}
	}
}

void add_vampiryzm(Line &l) {
	for (auto i : l.line) {
		if (liczba(i) > 0) {
			vampiryzm += liczba(i);
			break;
		}
	}
}

void add_health(Line &l) {
	int nmb = 0;
	for (auto i : l.line) {
		if (liczba(i) > 0) {
			nmb = liczba(i);
			break;
		}
	}
	if (nmb > 200)
		health_gained_by_potions += nmb;
	else
		health_gained_by_spells += nmb;
}

void add_potion(Line &l) {
	string name;
	bool activate = false;
	for (auto i : l.line) {
		if (activate) {
			if (name.size() == 0)
				name = i;
			else
				name = name + " " + i;
		}
		else {
			if (liczba(i) > 0 || i == "last")
				activate = true;
		}
	}
	if (name.back() == '.') name.pop_back();
	potions[name]++;
}

void add_exp(Line &l) {
	for (auto i : l.line) {
		if (liczba(i) > 0) {
			exp_gained += liczba(i);
			break;
		}
	}
}

void add_loot(Line &l) {
	bool activate = false;
	string str;
	int count = 0;
	if (l.line.back().back() != '.')
		l.line.back().push_back('.');
	for (auto i : l.line) {
		if (activate) {
			if (str.size() == 0 && count == 0) {
				if (liczba(i) > 0)
					count = liczba(i);
				else {
					count = 1;
					str = i;
				}
			}
			else
				str = str + " " + i;
			if (str.back() == ',' || str.back() == '.') {
				str.pop_back();
				if (str[0] == ' ') {
					int n = str.size();
					n--;
					str = str.substr(1, n);
				}
				loot[str] += count;
				str.clear();
				count = 0;
			}
		}
		else {
			if (i.back() == ':')
				activate = true;
		}
	}
}

const int doba = 24 * 60;
string print_interval_time(Time a, Time b) {
	int ta = a.h * 60 + a.m;
	int tb = b.h * 60 + b.m;
	int tm, th;

	if (b > a) {
		th = (tb - ta) / 60;
		tm = (tb - ta) % 60;
	}
	else {
		th = (doba - ta + tb) / 60;
		tm = (doba - ta + tb) % 60;
	}
	string ret = to_string(th) + " hours and " + to_string(tm) + " minutes";
	//cout << th << " hours and " << tm << " minutes";
	return ret;
}

void print_result() {
	cout << endl;
	cout << "Begin time: " << start_time.print_time() << endl;
	cout << "End time: " << end_time.print_time() << endl;
	cout << "Time: " << print_interval_time(start_time, end_time) << endl;
	cout << endl;

	cout << "--- Potions used ---" << endl;
	for (auto i : potions)
		cout << i.first << " -> " << i.second << endl;
	cout << endl;

	cout << "Health gained by vampiryzm " << vampiryzm << endl;
	cout << "Health gained by spells " << health_gained_by_spells << endl;
	cout << "Health gained by potions " << health_gained_by_potions << endl;
	cout << endl;

	cout << "Full damage " << full_damage << endl;
	cout << "Health losed " << health_losed << endl;
	cout << "Exp gained " << exp_gained << endl;
	cout << endl;

	cout << "--- Loot ---" << endl;
	for (auto i : loot)
		cout << i.first << " -> " << i.second << endl;
	cout << endl;
}

int main() {
	ios_base::sync_with_stdio(false); cin.tie(0);
	if (input_files()) {
		input_times();
		string tmp_read;
		while (getline(i_serverlog, tmp_read)) {
			if (tmp_read[0] < '0' || tmp_read[0] > '9') continue;
			int h = (tmp_read[0] - '0') * 10 + tmp_read[1] - '0';
			int m = (tmp_read[3] - '0') * 10 + tmp_read[4] - '0';
			Time tmp_time = Time(h, m);
			if (tmp_time < start_time || tmp_time > end_time) continue;
			int n = tmp_read.size();
			n -= 5;
			tmp_read = tmp_read.substr(6, n - 1);
			Line tmp_line;
			tmp_line.read_line(tmp_time, tmp_read);
			int rodzaj = check(tmp_line);
			if (rodzaj == SHOT_BY_PLAYER)
				add_player_damage(tmp_line);
			if (rodzaj == SHOT_BY_MONSTER)
				add_monster_damage(tmp_line);
			if (rodzaj == VAMPIRYZM)
				add_vampiryzm(tmp_line);
			if (rodzaj == HEAL_SPELL_POTION)
				add_health(tmp_line);
			if (rodzaj == USING_POTION)
				add_potion(tmp_line);
			if (rodzaj == EXP_GAINED)
				add_exp(tmp_line);
		}
		while (getline(i_loot, tmp_read)) {
			if (tmp_read[0] < '0' || tmp_read[0] > '9') continue;
			int h = (tmp_read[0] - '0') * 10 + tmp_read[1] - '0';
			int m = (tmp_read[3] - '0') * 10 + tmp_read[4] - '0';
			Time tmp_time = Time(h, m);
			if (tmp_time < start_time || tmp_time > end_time) continue;
			int n = tmp_read.size();
			n -= 5;
			tmp_read = tmp_read.substr(6, n - 1);
			Line tmp_line;
			tmp_line.read_line(tmp_time, tmp_read);
			int rodzaj = check(tmp_line);
			if (rodzaj == LOOT)
				add_loot(tmp_line);
		}
		print_result();
	}
	else {
		cout << "Cannot open files." << endl;
	}
	getchar(); getchar();
	return 0;
}

