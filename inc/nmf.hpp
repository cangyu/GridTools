#ifndef __NMF_HPP__
#define __NMF_HPP__

#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cctype>
#include <algorithm>
#include <cstddef>
#include <vector>
#include <array>
#include <string>
#include <map>
#include <set>
#include <utility>
#include <stdexcept>

namespace NMF
{
	template<typename T>
	class Array1D : public std::vector<T>
	{
	public:
		Array1D() : std::vector<T>() {}
		Array1D(size_t n) : std::vector<T>(n) {}
		Array1D(size_t n, const T &val) : std::vector<T>(n, val) {}
		Array1D(const Array1D &rhs) = default;
		~Array1D() = default;

		// 1-based indexing
		T &operator()(long long i)
		{
			const long long N = this->size();

			if (1 <= i && i <= N)
				return this->at(i - 1);
			else if (-N <= i && i <= -1)
				return this->at(N + i);
			else
				throw std::invalid_argument("\"" + std::to_string(i) + "\" is not a valid 1-based index.");
		}
		const T &operator()(long long i) const
		{
			const long long N = this->size();

			if (1 <= i && i <= N)
				return this->at(i - 1);
			else if (-N <= i && i <= -1)
				return this->at(N + i);
			else
				throw std::invalid_argument("\"" + std::to_string(i) + "\" is not a valid 1-based index.");
		}
	};

	static void str_formalize(std::string &s)
	{
		std::transform(s.begin(), s.end(), s.begin(), ::toupper);
		for (auto &e : s)
			if (e == '-')
				e = '_';
	}

	class BC
	{
	public:
		enum {
			COLLAPSED = 1,
			ONE_TO_ONE = 2,
			PATCHED = 3,
			POLE_DIR1 = 4,
			POLE_DIR2 = 5,
			SYM_X = 6,
			SYM_Y = 7,
			SYM_Z = 8,
			UNPROCESSED = 9,
			WALL = 10,
			SYM = 11,
			INFLOW = 12,
			OUTFLOW = 13
		};

		static bool isValidBCIdx(int x)
		{
			static const std::set<int> candidate_set{
				COLLAPSED,
				ONE_TO_ONE,
				PATCHED,
				POLE_DIR1,
				POLE_DIR2,
				SYM_X,
				SYM_Y,
				SYM_Z,
				UNPROCESSED,
				WALL,
				SYM,
				INFLOW,
				OUTFLOW
			};

			return candidate_set.find(x) != candidate_set.end();
		}

		static bool isValidBCStr(const std::string &x)
		{
			static const std::set<std::string> candidate_set{
				"COLLAPSED",
				"ONE_TO_ONE",
				"PATCHED",
				"POLE_DIR1", "POLE_DIR2",
				"SYM_X", "SYM_Y", "SYM_Z",
				"UNPROCESSED",
				"WALL",
				"SYM",
				"INFLOW",
				"OUTFLOW"
			};

			std::string x_(x);
			str_formalize(x_);
			return candidate_set.find(x_) != candidate_set.end();
		}

		static const std::string &idx2str(int x)
		{
			static const std::map<int, std::string> mapping_set{
				std::pair<int, std::string>(COLLAPSED, "COLLAPSED"),
				std::pair<int, std::string>(ONE_TO_ONE, "ONE_TO_ONE"),
				std::pair<int, std::string>(PATCHED, "PATCHED"),
				std::pair<int, std::string>(POLE_DIR1, "POLE_DIR1"),
				std::pair<int, std::string>(POLE_DIR2, "POLE_DIR2"),
				std::pair<int, std::string>(SYM_X, "SYM_X"),
				std::pair<int, std::string>(SYM_Y, "SYM_Y"),
				std::pair<int, std::string>(SYM_Z, "SYM_Z"),
				std::pair<int, std::string>(UNPROCESSED, "UNPROCESSED"),
				std::pair<int, std::string>(WALL, "WALL"),
				std::pair<int, std::string>(SYM, "SYM"),
				std::pair<int, std::string>(INFLOW, "INFLOW"),
				std::pair<int, std::string>(OUTFLOW, "OUTFLOW")
			};

			auto it = mapping_set.find(x);
			if (it == mapping_set.end())
				throw std::runtime_error("Not found!");
			else
				return it->second;
		}

		static int str2idx(const std::string &x)
		{
			static const std::map<std::string, int> mapping_set{
				std::pair<std::string, int>("COLLAPSED", COLLAPSED),
				std::pair<std::string, int>("ONE_TO_ONE", ONE_TO_ONE),
				std::pair<std::string, int>("PATCHED", PATCHED),
				std::pair<std::string, int>("POLE_DIR1", POLE_DIR1),
				std::pair<std::string, int>("POLE_DIR2", POLE_DIR2),
				std::pair<std::string, int>("SYM_X", SYM_X),
				std::pair<std::string, int>("SYM_Y", SYM_Y),
				std::pair<std::string, int>("SYM_Z", SYM_Z),
				std::pair<std::string, int>("UNPROCESSED", UNPROCESSED),
				std::pair<std::string, int>("WALL", WALL),
				std::pair<std::string, int>("SYM", SYM),
				std::pair<std::string, int>("SYMMETRY", SYM),
				std::pair<std::string, int>("INFLOW", INFLOW),
				std::pair<std::string, int>("OUTFLOW", OUTFLOW)
			};

			std::string x_(x);
			str_formalize(x_);

			auto it = mapping_set.find(x_);
			if (it == mapping_set.end())
				throw std::runtime_error("Not found!");
			else
				return it->second;
		}

		BC() = delete;
		~BC() = default;
	};

	class CELL
	{
	protected:
		size_t m_cell; // 1-based sequence
		Array1D<size_t> m_node; // 1-based sequence
		Array1D<size_t> m_face; // 1-based sequence

	public:
		CELL() = delete;
		CELL(bool is3D) : m_cell(0), m_node((is3D ? 8 : 4), 0), m_face((is3D ? 6 : 4), 0) {}
		CELL(const CELL &rhs) : m_cell(rhs.m_cell), m_node(rhs.m_node), m_face(rhs.m_face) {}
		virtual ~CELL() = default;

		size_t CellSeq() const { return m_cell; }
		size_t &CellSeq() { return m_cell; }

		// 1-based indexing of node
		size_t NodeSeq(int n) const { return m_node.at(n - 1); }
		size_t &NodeSeq(int n) { return m_node.at(n - 1); }

		// 1-based indexing of face
		size_t FaceSeq(int n) const { return m_face.at(n - 1); }
		size_t &FaceSeq(int n) { return m_face.at(n - 1); }
	};

	class QUAD_CELL : public CELL
	{
	public:
		QUAD_CELL() : CELL(false) {}
		QUAD_CELL(const QUAD_CELL &rhs) = default;
		~QUAD_CELL() = default;
	};

	class HEX_CELL : public CELL
	{
	public:
		HEX_CELL() : CELL(true) {}
		HEX_CELL(const HEX_CELL &rhs) = default;
		~HEX_CELL() = default;
	};

	class DIM
	{
	protected:
		bool m_is3D;
		int m_dim;

	public:
		DIM() = delete;
		DIM(int dim) : m_dim(dim)
		{
			if (dim == 2)
				m_is3D = false;
			else if (dim == 3)
				m_is3D = true;
			else
				throw std::invalid_argument("Invalid dimension: " + std::to_string(dim));
		}
		DIM(bool is3d) : m_is3D(is3d), m_dim(is3d ? 3 : 2) {}
		DIM(const DIM &rhs) = default;
		virtual ~DIM() = default;

		bool is3D() const { return m_is3D; }

		int dimension() const { return m_dim; }
	};

	class BLOCK : public DIM
	{
	protected:
		size_t idx; // 1-based global index
		size_t m_nI, m_nJ, m_nK; // Dimensions

	public:
		BLOCK() = delete;
		BLOCK(int nI, int nJ) :
			DIM(false),
			idx(0),
			m_nI(nI),
			m_nJ(nJ),
			m_nK(1)
		{
			if (nI < 2 || nJ < 2)
				throw std::invalid_argument("Invalid dimension.");
		}
		BLOCK(int nI, int nJ, int nK) :
			DIM(true),
			idx(0),
			m_nI(nI),
			m_nJ(nJ),
			m_nK(nK)
		{
			if (nI < 2 || nJ < 2 || nK < 2)
				throw std::invalid_argument("Invalid dimension.");
		}
		BLOCK(const BLOCK &rhs) = default;
		virtual ~BLOCK() = default;

		size_t index() const { return idx; }
		size_t &index() { return idx; }

		size_t IDIM() const { return m_nI; }
		size_t JDIM() const { return m_nJ; }
		size_t KDIM() const { return m_nK; }

		// Here the terms 'node', 'face' and 'cell' are 
		// consistent with that in ANSYS Fluent convention.
		size_t node_num() const
		{
			if (is3D())
				return IDIM() * JDIM() * KDIM();
			else
				return IDIM() * JDIM();
		}
		size_t face_num() const
		{
			size_t ret = 0;
			if (is3D())
			{
				ret += (IDIM() - 1) * JDIM() * KDIM();
				ret += IDIM() * (JDIM() - 1) * KDIM();
				ret += IDIM() * JDIM() * (KDIM() - 1);
			}
			else
			{
				ret += (IDIM() - 1) * JDIM();
				ret += IDIM() * (JDIM() - 1);
			}
			return ret;
		}
		size_t cell_num() const
		{
			if (is3D())
				return (IDIM() - 1) * (JDIM() - 1) * (KDIM() - 1);
			else
				return (IDIM() - 1) * (JDIM() - 1);
		}
	};

	class Block2D : public BLOCK
	{
	private:
		struct EDGE
		{
			short local_index = 0; // Ranges from 1 to 4, set to 0 when uninitialized.
			int global_index = 0; // 1-based global index, set to 0 when uninitialized.
			Block2D *dependentBlock = nullptr;
			Block2D *neighbourBlock = nullptr;
		};

	public:
		static const short NumOfEdge = 4;

		Block2D() = delete;
		Block2D(int nI, int nJ) :
			BLOCK(nI, nJ),
			m_cell(cell_num()),
			m_edge(NumOfEdge)
		{
			for (size_t i = 0; i < m_edge.size(); ++i)
				m_edge[i].local_index = i + 1;
		}
		Block2D(const Block2D &rhs) = delete;
		~Block2D() = default;

		// Access internal cell through 1-based index.
		// Indexing convention:
		//      "i" ranges from 1 to IDIM()-1;
		//      "j" ranges from 1 to JDIM()-1;
		// When the IJ-axis follows the right-hand convention, (i, j) represents
		// the left-most, bottom-most node of the selected cell.
		QUAD_CELL &cell(size_t i, size_t j)
		{
			const size_t i0 = i - 1, j0 = j - 1; // Convert 1-based index to 0-based
			const size_t idx = i0 + (m_nI - 1) * j0;
			return m_cell.at(idx);
		}
		const QUAD_CELL &cell(size_t i, size_t j) const
		{
			const size_t i0 = i - 1, j0 = j - 1; // Convert 1-based index to 0-based
			const size_t idx = i0 + (m_nI - 1) * j0;
			return m_cell.at(idx);
		}

		// Access the frame edges through 1-based index.
		// The indexing convention follows NMF specification.
		EDGE &edge(int n)
		{
			if (1 <= n && n <= NumOfEdge)
				return m_edge.at(n - 1);
			else if (-NumOfEdge <= n && n <= -1)
				return m_edge.at(NumOfEdge + n);
			else
				throw std::invalid_argument("\"" + std::to_string(n) + "\" is not a valid edge index for a 2D block.");
		}
		const EDGE &edge(int n) const
		{
			if (1 <= n && n <= NumOfEdge)
				return m_edge.at(n - 1);
			else if (-NumOfEdge <= n && n <= -1)
				return m_edge.at(NumOfEdge + n);
			else
				throw std::invalid_argument("\"" + std::to_string(n) + "\" is not a valid edge index for a 2D block.");
		}

	private:
		Array1D<QUAD_CELL> m_cell;
		Array1D<EDGE> m_edge;
	};

	class Block3D : public BLOCK
	{
	private:
		struct SURF;
		struct EDGE
		{
			short local_index = 0; // Ranges from 1 to 12, set to 0 when uninitialized.
			int global_index = 0; // 1-based global index, set to 0 when uninitialized.
			Block3D *dependentBlock = nullptr;
			std::array<SURF*, 2> dependentSurf{ nullptr, nullptr };
		};
		struct SURF
		{
			short local_index = 0; // Ranges from 1 to 6, set to 0 when uninitialized.
			std::array<EDGE*, 4> includedEdge{ nullptr, nullptr, nullptr, nullptr };
			Block3D *dependentBlock = nullptr;
			Block3D *neighbourBlock = nullptr;
			SURF *neighbourSurf = nullptr;
		};

	public:
		static const short NumOfEdge = 12;
		static const short NumOfSurf = 6;

		Block3D() = delete;
		Block3D(int nI, int nJ, int nK) :
			BLOCK(nI, nJ, nK),
			m_cell(cell_num()),
			m_edge(NumOfEdge),
			m_surf(NumOfSurf)
		{
			for (size_t i = 0; i < m_edge.size(); ++i)
			{
				auto &e = m_edge[i];
				e.local_index = i + 1;
				e.dependentBlock = this;
			}

			for (size_t i = 0; i < m_surf.size(); ++i)
			{
				auto &s = m_surf[i];
				s.local_index = i + 1;
				s.dependentBlock = this;
			}

			// Connection between frame edges and surrounding surfaces
			m_surf[0].includedEdge = { &m_edge[4], &m_edge[8], &m_edge[7], &m_edge[11] };
			m_surf[1].includedEdge = { &m_edge[5], &m_edge[10], &m_edge[6], &m_edge[9] };
			m_surf[2].includedEdge = { &m_edge[0], &m_edge[9], &m_edge[3], &m_edge[8] };
			m_surf[3].includedEdge = { &m_edge[1], &m_edge[11], &m_edge[2], &m_edge[10] };
			m_surf[4].includedEdge = { &m_edge[0], &m_edge[4], &m_edge[1], &m_edge[5] };
			m_surf[5].includedEdge = { &m_edge[2], &m_edge[7], &m_edge[3], &m_edge[6] };

			m_edge[0].dependentSurf = { &m_surf[2], &m_surf[4] };
			m_edge[1].dependentSurf = { &m_surf[4], &m_surf[3] };
			m_edge[2].dependentSurf = { &m_surf[3], &m_surf[5] };
			m_edge[3].dependentSurf = { &m_surf[5], &m_surf[2] };
			m_edge[4].dependentSurf = { &m_surf[0], &m_surf[4] };
			m_edge[5].dependentSurf = { &m_surf[4], &m_surf[1] };
			m_edge[6].dependentSurf = { &m_surf[1], &m_surf[5] };
			m_edge[7].dependentSurf = { &m_surf[5], &m_surf[0] };
			m_edge[8].dependentSurf = { &m_surf[0], &m_surf[2] };
			m_edge[9].dependentSurf = { &m_surf[2], &m_surf[1] };
			m_edge[10].dependentSurf = { &m_surf[1], &m_surf[3] };
			m_edge[11].dependentSurf = { &m_surf[3], &m_surf[0] };
		}
		Block3D(const Block3D &rhs) = delete;
		~Block3D() = default;

		// Access internal cell through 1-based index.
		// Indexing convention:
		//      "i" ranges from 1 to IDIM()-1;
		//      "j" ranges from 1 to JDIM()-1;
		//      "k" ranges from 1 to KDIM()-1;
		// When the IJK-axis follows the right-hand convention, (i, j, k) represents
		// the left-most, bottom-most and back-most node of the selected cell.
		HEX_CELL &cell(size_t i, size_t j, size_t k)
		{
			const size_t i0 = i - 1, j0 = j - 1, k0 = k - 1; // Convert 1-based index to 0-based
			const size_t idx = i0 + (m_nI - 1) * (j0 + (m_nJ - 1) * k0);
			return m_cell.at(idx);
		}
		const HEX_CELL &cell(size_t i, size_t j, size_t k) const
		{
			const size_t i0 = i - 1, j0 = j - 1, k0 = k - 1; // Convert 1-based index to 0-based
			const size_t idx = i0 + (m_nI - 1) * (j0 + (m_nJ - 1) * k0);
			return m_cell.at(idx);
		}

		// Access the frame edges through 1-based index.
		// The indexing convention follows NMF specification.
		EDGE &edge(int n)
		{
			if (1 <= n && n <= NumOfEdge)
				return m_edge.at(n - 1);
			else if (-NumOfEdge <= n && n <= -1)
				return m_edge.at(NumOfEdge + n);
			else
				throw std::invalid_argument("\"" + std::to_string(n) + "\" is not a valid edge index for a 3D block.");
		}
		const EDGE &edge(int n) const
		{
			if (1 <= n && n <= NumOfEdge)
				return m_edge.at(n - 1);
			else if (-NumOfEdge <= n && n <= -1)
				return m_edge.at(NumOfEdge + n);
			else
				throw std::invalid_argument("\"" + std::to_string(n) + "\" is not a valid edge index for a 3D block.");
		}

		// Access surrounding surface through 1-based index.
		// The index follows NMF convection.
		SURF &surf(int n)
		{
			if (1 <= n && n <= NumOfSurf)
				return m_surf.at(n - 1);
			else if (-NumOfSurf <= n && n <= -1)
				return m_surf.at(NumOfSurf + n);
			else
				throw std::invalid_argument("\"" + std::to_string(n) + "\" is not a valid 1-based surface index for a block.");
		}
		const SURF &surf(int n) const
		{
			if (1 <= n && n <= NumOfSurf)
				return m_surf.at(n - 1);
			else if (-NumOfSurf <= n && n <= -1)
				return m_surf.at(NumOfSurf + n);
			else
				throw std::invalid_argument("\"" + std::to_string(n) + "\" is not a valid 1-based surface index for a block.");
		}

	private:
		Array1D<HEX_CELL> m_cell;
		Array1D<EDGE> m_edge;
		Array1D<SURF> m_surf;
	};

	class RANGE
	{
	private:
		size_t m_blk; // Block index, 1-based.
		size_t m_face; // Face index, ranges from 1 to 6.
		size_t m_s1; // Primary direction starting index, 1-based.
		size_t m_e1; // Primary direction ending index, 1-based.
		size_t m_s2; // Secondary direction starting index, 1-based.
		size_t m_e2; // Secondary direction ending index, 1-based.

	public:
		RANGE() : m_blk(0), m_face(0), m_s1(0), m_e1(0), m_s2(0), m_e2(0) {}
		RANGE(size_t *src) : m_blk(src[0]), m_face(src[1]), m_s1(src[2]), m_e1(src[3]), m_s2(src[4]), m_e2(src[5]) {}
		RANGE(size_t b, size_t f, size_t s1, size_t e1, size_t s2, size_t e2) : m_blk(b), m_face(f), m_s1(s1), m_e1(e1), m_s2(s2), m_e2(e2) {}

		size_t B() const { return m_blk; }
		size_t &B() { return m_blk; }

		size_t F() const { return m_face; }
		size_t &F() { return m_face; }

		size_t S1() const { return m_s1; }
		size_t &S1() { return m_s1; }

		size_t E1() const { return m_e1; }
		size_t &E1() { return m_e1; }

		size_t S2() const { return m_s2; }
		size_t &S2() { return m_s2; }

		size_t E2() const { return m_e2; }
		size_t &E2() { return m_e2; }

		// Check if given index is within this range.
		bool constains(size_t pri, size_t sec) const
		{
			const bool t1 = (m_s1 <= pri) && (pri <= m_e1);
			const bool t2 = (m_s2 <= sec) && (sec <= m_e2);
			return t1 && t2;
		}

		// Nodes in primary direction.
		size_t pri_node_num() const { return m_e1 - m_s1 + 1; }

		// Nodes in secondary direction.
		size_t sec_node_num() const { return m_e2 - m_s2 + 1; }

		// Total nodes on this interface.
		size_t node_num() const { return pri_node_num() * sec_node_num(); }

		// Total edges on this interface.
		size_t edge_num() const
		{
			const size_t n_pri = (pri_node_num() - 1) * sec_node_num();
			const size_t n_sec = (sec_node_num() - 1) * pri_node_num();
			return n_pri + n_sec;
		}

		// Total quad cells on this interface.
		size_t face_num() const { return (pri_node_num() - 1) * (sec_node_num() - 1); }
	};

	class ENTRY
	{
	private:
		int m_bc;
		RANGE *m_rg1, *m_rg2;
		bool m_swap;

	public:
		ENTRY(const std::string &t, size_t *s) : m_rg1(new RANGE(s)), m_rg2(nullptr), m_swap(false)
		{
			if (BC::isValidBCStr(t))
				m_bc = BC::str2idx(t);
			else
				throw std::runtime_error("Unsupported B.C. name: \"" + t + "\"");
		}
		ENTRY(const std::string &t, size_t *s1, size_t *s2, bool f) : m_rg1(new RANGE(s1)), m_rg2(new RANGE(s2)), m_swap(f)
		{
			if (BC::isValidBCStr(t))
				m_bc = BC::str2idx(t);
			else
				throw std::runtime_error("Unsupported B.C. name: \"" + t + "\"");
		}
		~ENTRY()
		{
			if (m_rg1)
				delete m_rg1;
			if (m_rg2)
				delete m_rg2;
		}

		int Type() const { return m_bc; }
		int &Type() { return m_bc; }

		size_t B1() const { return m_rg1->B(); }
		size_t &B1() { return m_rg1->B(); }

		size_t F1() const { return m_rg1->F(); }
		size_t &F1() { return m_rg1->F(); }

		size_t B2() const { return m_rg2->B(); }
		size_t &B2() { return m_rg2->B(); }

		size_t F2() const { return m_rg2->F(); }
		size_t &F2() { return m_rg2->F(); }

		const RANGE &Range1() const { return *m_rg1; }
		RANGE &Range1() { return *m_rg1; }

		const RANGE &Range2() const { return *m_rg2; }
		RANGE &Range2() { return *m_rg2; }

		bool Swap() const { return m_swap; }
		bool &Swap() { return m_swap; }

		size_t node_num() const { return m_rg1->node_num(); }

		size_t face_num() const { return m_rg1->face_num(); }

		int contains(size_t bs, size_t fs, size_t lpri, size_t lsec) const
		{
			// Left
			if (B1() == bs && F1() == fs && Range1().constains(lpri, lsec))
				return 1;
			else if (m_rg2 && B2() == bs && F2() == fs && Range2().constains(lpri, lsec))
				return 2;
			else
				return 0;
		}
	};

	class Mapping3D
	{
	private:
		Array1D<Block3D> m_blk;
		Array1D<ENTRY> m_entry;

		void coloring_block_frame()
		{
			for (auto &blk : m_blk)
			{
				// TODO
			}
		}

	public:
		Mapping3D() = default;
		~Mapping3D() = default;

		size_t nBlk() const { return m_blk.size(); }

		int readFromFile(const std::string &path)
		{
			std::string s;
			std::stringstream ss;

			//Load file
			std::ifstream mfp(path);
			if (mfp.fail())
				throw std::runtime_error("Can not open target input file: \"" + path + "\".");

			//Skip header
			do {
				std::getline(mfp, s, '\n');
			} while (s.find('#') != std::string::npos);

			//Read block dimension info
			size_t NumOfBlk;
			ss << s;
			ss >> NumOfBlk;
			if (NumOfBlk == 0)
				throw std::runtime_error("Invalid num of blocks: " + std::to_string(NumOfBlk));

			m_blk.clear();
			for (size_t i = 1; i <= NumOfBlk; i++)
			{
				size_t idx, i_max, j_max, k_max;
				std::getline(mfp, s, '\n');
				ss.clear();
				ss << s;
				ss >> idx >> i_max >> j_max >> k_max;

				if (idx != i)
					throw std::runtime_error("Invalid order of block: " + std::to_string(idx));
				if (i_max == 0)
					throw std::runtime_error("Invalid I dimension: " + std::to_string(i_max));
				if (j_max == 0)
					throw std::runtime_error("Invalid J dimension: " + std::to_string(j_max));
				if (k_max == 0)
					throw std::runtime_error("Invalid K dimension: " + std::to_string(k_max));

				m_blk.emplace_back(i_max, j_max, k_max);
			}

			//Skip separators
			do {
				std::getline(mfp, s, '\n');
			} while (s.find('#') != std::string::npos);

			//Read bc mappings
			m_entry.clear();
			do {
				str_formalize(s);
				ss.clear();
				ss << s;
				std::string bc_str;
				ss >> bc_str;
				size_t connectivity[2][6] = { 0 };
				if (BC::str2idx(bc_str) == BC::ONE_TO_ONE)
				{
					for (int i = 0; i < 2; i++)
						for (int j = 0; j < 6; j++)
							ss >> connectivity[i][j];

					std::string swp;
					ss >> swp;

					m_entry.emplace_back(bc_str, connectivity[0], connectivity[1], swp == "TRUE");
				}
				else
				{
					for (int i = 0; i < 6; i++)
						ss >> connectivity[0][i];

					m_entry.emplace_back(bc_str, connectivity[0]);
				}
			} while (std::getline(mfp, s, '\n'));

			// Finalize
			mfp.close();
			return 0;
		}

		int writeToFile(const std::string &path)
		{
			// Open target file
			std::ofstream f_out(path);
			if (f_out.fail())
				throw std::runtime_error("Can not open target output file: " + path);

			// Header
			f_out << "# ======================== Neutral Map File generated by the Grid-Glue software ==============================" << std::endl;
			f_out << "# ============================================================================================================" << std::endl;
			f_out << "# Block#    IDIM    JDIM    KDIM" << std::endl;
			f_out << "# ------------------------------------------------------------------------------------------------------------" << std::endl;

			// Block info
			const size_t NumOfBlk = nBlk();
			f_out << std::setw(8) << std::right << NumOfBlk << std::endl;
			for (size_t i = 0; i < NumOfBlk; i++)
			{
				f_out << std::setw(8) << std::right << i + 1;
				f_out << std::setw(8) << std::right << m_blk[i].IDIM();
				f_out << std::setw(8) << std::right << m_blk[i].JDIM();
				f_out << std::setw(8) << std::right << m_blk[i].KDIM();
				f_out << std::endl;
			}

			// Interface and boundary info
			f_out << "# ============================================================================================================" << std::endl;
			f_out << "# Type           B1    F1       S1    E1       S2    E2       B2    F2       S1    E1       S2    E2      Swap" << std::endl;
			f_out << "# ------------------------------------------------------------------------------------------------------------" << std::endl;
			for (auto & e : m_entry)
			{
				f_out << std::setw(13) << std::left << BC::idx2str(e.Type());
				f_out << std::setw(6) << std::right << e.B1();
				f_out << std::setw(6) << std::right << e.F1();
				f_out << std::setw(9) << std::right << e.Range1().S1();
				f_out << std::setw(6) << std::right << e.Range1().E1();
				f_out << std::setw(9) << std::right << e.Range1().S2();
				f_out << std::setw(6) << std::right << e.Range1().E2();
				if (e.Type() == BC::ONE_TO_ONE)
				{
					f_out << std::setw(9) << std::right << e.B2();
					f_out << std::setw(6) << std::right << e.F2();
					f_out << std::setw(9) << std::right << e.Range2().S1();
					f_out << std::setw(6) << std::right << e.Range2().E1();
					f_out << std::setw(9) << std::right << e.Range2().S2();
					f_out << std::setw(6) << std::right << e.Range2().E2();
					f_out << std::setw(10) << std::right << (e.Swap() ? "TRUE" : "FALSE");
				}
				f_out << std::endl;
			}

			// Finalize
			f_out.close();
			return 0;
		}

		size_t nCell() const
		{
			size_t ret = 0;
			for (const auto & blk : m_blk)
				ret += blk.cell_num();
			return ret;
		}

		size_t nFace() const
		{
			size_t ret = 0;
			for (const auto &blk : m_blk)
				ret += blk.cell_num();

			// Sustract duplicated interface
			for (const auto &e : m_entry)
				if (e.Type() == BC::ONE_TO_ONE)
					ret -= e.face_num();

			return ret;
		}

		int compute_topology()
		{
			// Indexing of cells
			size_t cnt = 0;
			for (auto &blk : m_blk)
			{
				const size_t cI = blk.IDIM();
				const size_t cJ = blk.JDIM();
				const size_t cK = blk.KDIM();

				for (size_t k = 1; k < cK; ++k)
					for (size_t j = 1; j < cJ; ++j)
						for (size_t i = 1; i < cI; ++i)
							blk.cell(i, j, k).CellSeq() = ++cnt;
			}

			const size_t totalCellNum = nCell();
			if (cnt != totalCellNum)
				throw std::length_error("Inconsistent num of cells detected.");

			// Indexing of faces
			cnt = 0;
			for (auto &blk : m_blk)
			{
				const size_t cI = blk.IDIM();
				const size_t cJ = blk.JDIM();
				const size_t cK = blk.KDIM();

				// TODO
			}
			const size_t totalFaceNum = nFace();
			if (cnt != totalFaceNum)
				throw std::length_error("Inconsistent num of cells detected.");

			// Indexing of nodes
			cnt = 0;
			// TODO

			return 0;
		}
	};
}

#endif
