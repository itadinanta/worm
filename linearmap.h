#ifndef _LINEARMAP_H
#define _LINEARMAP_H

#include <math.h>
#include <vector>
#include <stdio.h>

namespace spline {	
	using namespace std;

	template <class KFLOAT = float, class VFLOAT = float>
	class LinearMap {
		class Item {
		public:
			KFLOAT key; 
			VFLOAT value;
			Item(KFLOAT key, VFLOAT value) {
				this->key = key;
				this->value = value;
			}
		};
		vector<Item> values;
	protected:
		const int k;
	public:
		LinearMap(const int K=1): k(K) {};
		const VFLOAT eval(KFLOAT t, int first) const {
			const Item *F, *L, *M;
			if (first>=values.size()-k) first=values.size()-k-1;
			if (first<k-1) first = k-1;
			F = &values[first];
			if ((first+1)>=values.size()) return F->value;
			L = &values[first+1];
			return  F->value + 
				(t - F->key) * (L->value - F->value) 
				/ (L->key - F->key);
		}
		const VFLOAT eval(KFLOAT t) const {			
			int first = firstNotLess(t);
			return eval(t, first);
		}
		int firstNotLess(KFLOAT t) const {
			return firstNotLess(t, k-1, values.size()-k);
		}
		int firstNotLess(KFLOAT t, int first, int last) const {
			int mid;
			const Item *F, *L, *M1, *M2;
			F = &values[first];
			L = &values[last];	
//			printf("--looking for %f[%i, %i]\n",t,first,last);
			while (first<last-1) {
				mid = (first + last) >> 1;
				M1 = &values[mid];
				M2 = &values[mid+1];
//				printf("\t(%f-%f) (%f-%f) (%d-%d) (%d-%d)\n", F->key, M1->key, M2->key, L->key, first, mid, mid+1, last);
				if (t <= F->key)
					return first;
				else if (t >= L->key)
					return last;
				else if (t < M1->key) {
					last = mid;
					L = M1;
				}
				else if (t > M2->key) {
					first = mid;
					F = M2;
				}
				else {
					first = mid;
					break;
				}
			}
			return first;
		}
		const VFLOAT operator() (KFLOAT f) const {
			return eval(f);
		};
		const VFLOAT operator[] (int i) const {
			return values[i].key;
		};
		void add(KFLOAT key, VFLOAT value) {
			values.push_back(Item(key, value));
		};
		void add(KFLOAT key) {
			values.push_back(Item(key, values.size()));
		};
		void clear() {
			values.clear();
		};
	}; 
}

#endif
