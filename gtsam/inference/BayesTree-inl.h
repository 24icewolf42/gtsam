/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    BayesTree.cpp
 * @brief   Bayes Tree is a tree of cliques of a Bayes Chain
 * @author  Frank Dellaert
 * @author  Michael Kaess
 * @author  Viorela Ila
 * @author  Richard Roberts
 */

#pragma once

#include <gtsam/base/FastSet.h>
#include <gtsam/base/FastVector.h>
#include <gtsam/inference/BayesTree.h>
#include <gtsam/inference/inference.h>
#include <gtsam/inference/GenericSequentialSolver.h>

#include <iostream>
#include <algorithm>

#include <boost/foreach.hpp>
#include <boost/assign/std/list.hpp> // for operator +=
#include <boost/format.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <fstream>
using namespace boost::assign;
namespace lam = boost::lambda;

namespace gtsam {

	using namespace std;

	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	typename BayesTree<CONDITIONAL,CLIQUE>::CliqueData
	BayesTree<CONDITIONAL,CLIQUE>::getCliqueData() const {
		CliqueData data;
		getCliqueData(data, root_);
		return data;
	}

	template<class CONDITIONAL, class CLIQUE>
	void BayesTree<CONDITIONAL,CLIQUE>::getCliqueData(CliqueData& data,
			BayesTree<CONDITIONAL,CLIQUE>::sharedClique clique) const {
		data.conditionalSizes.push_back((*clique)->nrFrontals());
		data.separatorSizes.push_back((*clique)->nrParents());
		BOOST_FOREACH(sharedClique c, clique->children_) {
			getCliqueData(data, c);
		}
	}

	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	void BayesTree<CONDITIONAL,CLIQUE>::saveGraph(const std::string &s) const {
		if (!root_.get()) throw invalid_argument("the root of bayes tree has not been initialized!");
		ofstream of(s.c_str());
		of<< "digraph G{\n";
		saveGraph(of, root_);
		of<<"}";
		of.close();
	}

	template<class CONDITIONAL, class CLIQUE>
	void BayesTree<CONDITIONAL,CLIQUE>::saveGraph(ostream &s,
			BayesTree<CONDITIONAL,CLIQUE>::sharedClique clique,
			int parentnum) const {
		static int num = 0;
		bool first = true;
		std::stringstream out;
		out << num;
		string parent = out.str();
		parent += "[label=\"";

		BOOST_FOREACH(boost::shared_ptr<CONDITIONAL> c, clique->conditionals_) {
			if(!first) parent += ","; first = false;
			parent += (string(c->key())).c_str();
		}

		if( clique != root_){
			parent += " : ";
			s << parentnum << "->" << num << "\n";
		}

		first = true;
		BOOST_FOREACH(Index sep, clique->separator_) {
			if(!first) parent += ","; first = false;
			parent += (boost::format("%1%")%sep).str();
		}
		parent += "\"];\n";
		s << parent;
		parentnum = num;

		BOOST_FOREACH(sharedClique c, clique->children_) {
			num++;
			saveGraph(s, c, parentnum);
		}
	}


	template<class CONDITIONAL, class CLIQUE>
	typename BayesTree<CONDITIONAL,CLIQUE>::CliqueStats
	BayesTree<CONDITIONAL,CLIQUE>::CliqueData::getStats() const {
		CliqueStats stats;

		double sum = 0.0;
		size_t max = 0;
		BOOST_FOREACH(size_t s, conditionalSizes) {
			sum += (double)s;
			if(s > max) max = s;
		}
		stats.avgConditionalSize = sum / (double)conditionalSizes.size();
		stats.maxConditionalSize = max;

		sum = 0.0;
		max = 1;
		BOOST_FOREACH(size_t s, separatorSizes) {
			sum += (double)s;
			if(s > max) max = s;
		}
		stats.avgSeparatorSize = sum / (double)separatorSizes.size();
		stats.maxSeparatorSize = max;

		return stats;
	}

	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	void BayesTree<CONDITIONAL,CLIQUE>::Cliques::print(const std::string& s) const {
		cout << s << ":\n";
		BOOST_FOREACH(sharedClique clique, *this)
				clique->printTree();
	}

	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	bool BayesTree<CONDITIONAL,CLIQUE>::Cliques::equals(const Cliques& other, double tol) const {
		return other == *this;
	}

	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	typename BayesTree<CONDITIONAL,CLIQUE>::sharedClique
	BayesTree<CONDITIONAL,CLIQUE>::addClique(const sharedConditional& conditional, const sharedClique& parent_clique) {
		sharedClique new_clique(new Clique(conditional));
		addClique(new_clique, parent_clique);
		return new_clique;
	}

  /* ************************************************************************* */
  template<class CONDITIONAL, class CLIQUE>
	void BayesTree<CONDITIONAL,CLIQUE>::addClique(const sharedClique& clique, const sharedClique& parent_clique) {
    nodes_.resize(std::max((*clique)->lastFrontalKey()+1, nodes_.size()));
    BOOST_FOREACH(Index key, (*clique)->frontals())
      nodes_[key] = clique;
    if (parent_clique != NULL) {
      clique->parent_ = parent_clique;
      parent_clique->children_.push_back(clique);
    } else {
      assert(!root_);
      root_ = clique;
    }
    clique->assertInvariants();
  }

  /* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	typename BayesTree<CONDITIONAL,CLIQUE>::sharedClique BayesTree<CONDITIONAL,CLIQUE>::addClique(
	    const sharedConditional& conditional, list<sharedClique>& child_cliques) {
		sharedClique new_clique(new Clique(conditional));
		nodes_.resize(std::max(conditional->lastFrontalKey()+1, nodes_.size()));
		BOOST_FOREACH(Index key, conditional->frontals())
		  nodes_[key] = new_clique;
		new_clique->children_ = child_cliques;
		BOOST_FOREACH(sharedClique& child, child_cliques)
			child->parent_ = new_clique;
		new_clique->assertInvariants();
		return new_clique;
	}

  /* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	inline void BayesTree<CONDITIONAL,CLIQUE>::addToCliqueFront(BayesTree<CONDITIONAL,CLIQUE>& bayesTree, const sharedConditional& conditional, const sharedClique& clique) {
	  static const bool debug = false;
#ifndef NDEBUG
	  // Debug check to make sure the conditional variable is ordered lower than
	  // its parents and that all of its parents are present either in this
	  // clique or its separator.
	  BOOST_FOREACH(Index parent, conditional->parents()) {
	    assert(parent > conditional->lastFrontalKey());
	    const Clique& cliquer(*clique);
	    assert(find(cliquer->begin(), cliquer->end(), parent) != cliquer->end());
	  }
#endif
	  if(debug) conditional->print("Adding conditional ");
	  if(debug) clique->print("To clique ");
	  Index key = conditional->lastFrontalKey();
	  bayesTree.nodes_.resize(std::max(key+1, bayesTree.nodes_.size()));
	  bayesTree.nodes_[key] = clique;
	  FastVector<Index> newKeys((*clique)->size() + 1);
	  newKeys[0] = key;
	  std::copy((*clique)->begin(), (*clique)->end(), newKeys.begin()+1);
	  clique->conditional_ = CONDITIONAL::FromKeys(newKeys, (*clique)->nrFrontals() + 1);
	  if(debug) clique->print("Expanded clique is ");
	  clique->assertInvariants();
	}

	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	void BayesTree<CONDITIONAL,CLIQUE>::removeClique(sharedClique clique) {

		if (clique->isRoot())
			root_.reset();
		else // detach clique from parent
	    clique->parent_.lock()->children_.remove(clique);

	  // orphan my children
		BOOST_FOREACH(sharedClique child, clique->children_)
	  	child->parent_ = typename Clique::weak_ptr();

	  BOOST_FOREACH(Index key, (*clique->conditional())) {
			nodes_[key].reset();
	  }
	}

  /* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	void BayesTree<CONDITIONAL,CLIQUE>::recursiveTreeBuild(const boost::shared_ptr<BayesTreeClique<IndexConditional> >& symbolic,
	    const std::vector<boost::shared_ptr<CONDITIONAL> >& conditionals,
	    const typename BayesTree<CONDITIONAL,CLIQUE>::sharedClique& parent) {

	  // Helper function to build a non-symbolic tree (e.g. Gaussian) using a
	  // symbolic tree, used in the BT(BN) constructor.

	  // Build the current clique
	  FastList<typename CONDITIONAL::shared_ptr> cliqueConditionals;
	  BOOST_FOREACH(Index j, symbolic->conditional()->frontals()) {
	    cliqueConditionals.push_back(conditionals[j]); }
	  typename BayesTree<CONDITIONAL,CLIQUE>::sharedClique thisClique(new CLIQUE(CONDITIONAL::Combine(cliqueConditionals.begin(), cliqueConditionals.end())));

	  // Add the new clique with the current parent
	  this->addClique(thisClique, parent);

	  // Build the children, whose parent is the new clique
	  BOOST_FOREACH(const BayesTree<IndexConditional>::sharedClique& child, symbolic->children()) {
	    this->recursiveTreeBuild(child, conditionals, thisClique); }
	}

  /* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	BayesTree<CONDITIONAL,CLIQUE>::BayesTree(const BayesNet<CONDITIONAL>& bayesNet) {
	  // First generate symbolic BT to determine clique structure
	  BayesTree<IndexConditional> sbt(bayesNet);

	  // Build index of variables to conditionals
	  std::vector<boost::shared_ptr<CONDITIONAL> > conditionals(sbt.root()->conditional()->frontals().back() + 1);
	  BOOST_FOREACH(const boost::shared_ptr<CONDITIONAL>& c, bayesNet) {
	    if(c->nrFrontals() != 1)
	      throw std::invalid_argument("BayesTree constructor from BayesNet only supports single frontal variable conditionals");
	    if(c->firstFrontalKey() >= conditionals.size())
	      throw std::invalid_argument("An inconsistent BayesNet was passed into the BayesTree constructor!");
	    if(conditionals[c->firstFrontalKey()])
	      throw std::invalid_argument("An inconsistent BayesNet with duplicate frontal variables was passed into the BayesTree constructor!");

	    conditionals[c->firstFrontalKey()] = c;
	  }

	  // Build the new tree
	  this->recursiveTreeBuild(sbt.root(), conditionals, sharedClique());
	}

	/* ************************************************************************* */
	template<>
	inline BayesTree<IndexConditional>::BayesTree(const BayesNet<IndexConditional>& bayesNet) {
		BayesNet<IndexConditional>::const_reverse_iterator rit;
		for ( rit=bayesNet.rbegin(); rit != bayesNet.rend(); ++rit )
			insert(*this, *rit);
	}

	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	BayesTree<CONDITIONAL,CLIQUE>::BayesTree(const BayesNet<CONDITIONAL>& bayesNet, std::list<BayesTree<CONDITIONAL,CLIQUE> > subtrees) {
		if (bayesNet.size() == 0)
			throw invalid_argument("BayesTree::insert: empty bayes net!");

		// get the roots of child subtrees and merge their nodes_
		list<sharedClique> childRoots;
		typedef BayesTree<CONDITIONAL,CLIQUE> Tree;
		BOOST_FOREACH(const Tree& subtree, subtrees) {
			nodes_.insert(subtree.nodes_.begin(), subtree.nodes_.end());
			childRoots.push_back(subtree.root());
		}

		// create a new clique and add all the conditionals to the clique
		sharedClique new_clique;
		typename BayesNet<CONDITIONAL>::sharedConditional conditional;
		BOOST_REVERSE_FOREACH(conditional, bayesNet) {
			if (!new_clique.get())
				new_clique = addClique(conditional,childRoots);
			else
			  addToCliqueFront(*this, conditional, new_clique);
		}

		root_ = new_clique;
	}

  /* ************************************************************************* */
  template<class CONDITIONAL, class CLIQUE>
  BayesTree<CONDITIONAL,CLIQUE>::BayesTree(const This& other) {
    *this = other;
  }

  /* ************************************************************************* */
  template<class CONDITIONAL, class CLIQUE>
  BayesTree<CONDITIONAL,CLIQUE>& BayesTree<CONDITIONAL,CLIQUE>::operator=(const This& other) {
    this->clear();
    other.cloneTo(*this);
    return *this;
  }

	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	void BayesTree<CONDITIONAL,CLIQUE>::print(const string& s) const {
		if (root_.use_count() == 0) {
			printf("WARNING: BayesTree.print encountered a forest...\n");
			return;
		}
		cout << s << ": clique size == " << size() << ", node size == " << nodes_.size() <<  endl;
		if (nodes_.empty()) return;
		root_->printTree("");
	}

	/* ************************************************************************* */
	// binary predicate to test equality of a pair for use in equals
	template<class CONDITIONAL, class CLIQUE>
	bool check_sharedCliques(
			const typename BayesTree<CONDITIONAL,CLIQUE>::sharedClique& v1,
			const typename BayesTree<CONDITIONAL,CLIQUE>::sharedClique& v2
	) {
		return (!v1 && !v2) || (v1 && v2 && v1->equals(*v2));
	}

	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	bool BayesTree<CONDITIONAL,CLIQUE>::equals(const BayesTree<CONDITIONAL,CLIQUE>& other,
			double tol) const {
		return size()==other.size() &&
				std::equal(nodes_.begin(), nodes_.end(), other.nodes_.begin(), &check_sharedCliques<CONDITIONAL,CLIQUE>);
	}

	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	template<class CONTAINER>
	inline Index BayesTree<CONDITIONAL,CLIQUE>::findParentClique(const CONTAINER& parents) const {
	  typename CONTAINER::const_iterator lowestOrderedParent = min_element(parents.begin(), parents.end());
	  assert(lowestOrderedParent != parents.end());
	  return *lowestOrderedParent;
	}

	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	void BayesTree<CONDITIONAL,CLIQUE>::insert(BayesTree<CONDITIONAL,CLIQUE>& bayesTree, const sharedConditional& conditional)
	{
	  static const bool debug = false;

		// get key and parents
		const typename CONDITIONAL::Parents& parents = conditional->parents();

		if(debug) conditional->print("Adding conditional ");

		// if no parents, start a new root clique
		if (parents.empty()) {
		  if(debug) cout << "No parents so making root" << endl;
		  bayesTree.root_ = bayesTree.addClique(conditional);
			return;
		}

		// otherwise, find the parent clique by using the index data structure
		// to find the lowest-ordered parent
		Index parentRepresentative = bayesTree.findParentClique(parents);
		if(debug) cout << "First-eliminated parent is " << parentRepresentative << ", have " << bayesTree.nodes_.size() << " nodes." << endl;
		sharedClique parent_clique = bayesTree[parentRepresentative];
		if(debug) parent_clique->print("Parent clique is ");

		// if the parents and parent clique have the same size, add to parent clique
		if ((*parent_clique)->size() == size_t(parents.size())) {
		  if(debug) cout << "Adding to parent clique" << endl;
#ifndef NDEBUG
		  // Debug check that the parent keys of the new conditional match the keys
		  // currently in the clique.
//		  list<Index>::const_iterator parent = parents.begin();
//		  typename Clique::const_iterator cond = parent_clique->begin();
//		  while(parent != parents.end()) {
//		    assert(cond != parent_clique->end());
//		    assert(*parent == (*cond)->key());
//		    ++ parent;
//		    ++ cond;
//		  }
#endif
		  addToCliqueFront(bayesTree, conditional, parent_clique);
		} else {
		  if(debug) cout << "Starting new clique" << endl;
		  // otherwise, start a new clique and add it to the tree
		  bayesTree.addClique(conditional,parent_clique);
		}
	}

	/* ************************************************************************* */
	//TODO: remove this function after removing TSAM.cpp
	template<class CONDITIONAL, class CLIQUE>
	typename BayesTree<CONDITIONAL,CLIQUE>::sharedClique BayesTree<CONDITIONAL,CLIQUE>::insert(
	    const sharedConditional& clique, list<sharedClique>& children, bool isRootClique) {

		if (clique->nrFrontals() == 0)
			throw invalid_argument("BayesTree::insert: empty clique!");

		// create a new clique and add all the conditionals to the clique
		sharedClique new_clique = addClique(clique, children);
		if (isRootClique) root_ = new_clique;

		return new_clique;
	}

  /* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	void BayesTree<CONDITIONAL,CLIQUE>::fillNodesIndex(const sharedClique& subtree) {
	  // Add each frontal variable of this root node
	  BOOST_FOREACH(const Index& key, subtree->conditional()->frontals()) { nodes_[key] = subtree; }
	  // Fill index for each child
	  typedef typename BayesTree<CONDITIONAL,CLIQUE>::sharedClique sharedClique;
	  BOOST_FOREACH(const sharedClique& child, subtree->children_) {
	    fillNodesIndex(child); }
	}

  /* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	void BayesTree<CONDITIONAL,CLIQUE>::insert(const sharedClique& subtree) {
	  if(subtree) {
	    // Find the parent clique of the new subtree.  By the running intersection
	    // property, those separator variables in the subtree that are ordered
	    // lower than the highest frontal variable of the subtree root will all
	    // appear in the separator of the subtree root.
	    if(subtree->conditional()->parents().empty()) {
	      assert(!root_);
	      root_ = subtree;
	    } else {
	      Index parentRepresentative = findParentClique(subtree->conditional()->parents());
	      sharedClique parent = (*this)[parentRepresentative];
	      parent->children_ += subtree;
	      subtree->parent_ = parent; // set new parent!
	    }

	    // Now fill in the nodes index
	    if(nodes_.size() == 0 ||
	        *std::max_element(subtree->conditional()->beginFrontals(), subtree->conditional()->endFrontals()) > (nodes_.size() - 1)) {
	      nodes_.resize(subtree->conditional()->lastFrontalKey() + 1);
	    }
	    fillNodesIndex(subtree);
	  }
	}

	/* ************************************************************************* */
	// First finds clique marginal then marginalizes that
	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	typename CONDITIONAL::FactorType::shared_ptr BayesTree<CONDITIONAL,CLIQUE>::marginalFactor(
			Index key, Eliminate function) const {

		// get clique containing key
		sharedClique clique = (*this)[key];

		// calculate or retrieve its marginal
		FactorGraph<FactorType> cliqueMarginal = clique->marginal(root_,function);

		return GenericSequentialSolver<FactorType>(cliqueMarginal).marginalFactor(
				key, function);
	}

	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	typename BayesNet<CONDITIONAL>::shared_ptr BayesTree<CONDITIONAL,CLIQUE>::marginalBayesNet(
			Index key, Eliminate function) const {

	  // calculate marginal as a factor graph
	  FactorGraph<FactorType> fg;
	  fg.push_back(this->marginalFactor(key,function));

		// eliminate factor graph marginal to a Bayes net
		return GenericSequentialSolver<FactorType>(fg).eliminate(function);
	}

	/* ************************************************************************* */
	// Find two cliques, their joint, then marginalizes
	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	typename FactorGraph<typename CONDITIONAL::FactorType>::shared_ptr
	BayesTree<CONDITIONAL,CLIQUE>::joint(Index key1, Index key2, Eliminate function) const {

		// get clique C1 and C2
		sharedClique C1 = (*this)[key1], C2 = (*this)[key2];

		// calculate joint
		FactorGraph<FactorType> p_C1C2(C1->joint(C2, root_, function));

		// eliminate remaining factor graph to get requested joint
		vector<Index> key12(2); key12[0] = key1; key12[1] = key2;
		GenericSequentialSolver<FactorType> solver(p_C1C2);
		return solver.jointFactorGraph(key12,function);
	}

	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	typename BayesNet<CONDITIONAL>::shared_ptr BayesTree<CONDITIONAL,CLIQUE>::jointBayesNet(
			Index key1, Index key2, Eliminate function) const {

		// eliminate factor graph marginal to a Bayes net
		return GenericSequentialSolver<FactorType> (
				*this->joint(key1, key2, function)).eliminate(function);
	}

	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	void BayesTree<CONDITIONAL,CLIQUE>::clear() {
		// Remove all nodes and clear the root pointer
		nodes_.clear();
		root_.reset();
	}

	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
	void BayesTree<CONDITIONAL,CLIQUE>::removePath(sharedClique clique,
			BayesNet<CONDITIONAL>& bn, typename BayesTree<CONDITIONAL,CLIQUE>::Cliques& orphans) {

		// base case is NULL, if so we do nothing and return empties above
		if (clique!=NULL) {

			// remove the clique from orphans in case it has been added earlier
			orphans.remove(clique);

			// remove me
			this->removeClique(clique);

			// remove path above me
			this->removePath(typename Clique::shared_ptr(clique->parent_.lock()), bn, orphans);

			// add children to list of orphans (splice also removed them from clique->children_)
			orphans.splice (orphans.begin(), clique->children_);

			bn.push_back(clique->conditional());

		}
	}

	/* ************************************************************************* */
	template<class CONDITIONAL, class CLIQUE>
  template<class CONTAINER>
  void BayesTree<CONDITIONAL,CLIQUE>::removeTop(const CONTAINER& keys,
  		BayesNet<CONDITIONAL>& bn, typename BayesTree<CONDITIONAL,CLIQUE>::Cliques& orphans) {

		// process each key of the new factor
	  BOOST_FOREACH(const Index& key, keys) {

	    // get the clique
	    if(key < nodes_.size()) {
	      const sharedClique& clique(nodes_[key]);
	      if(clique) {
	        // remove path from clique to root
	        this->removePath(clique, bn, orphans);
	      }
	    }
	  }
	}

	/* ************************************************************************* */
  template<class CONDITIONAL, class CLIQUE>
	void BayesTree<CONDITIONAL,CLIQUE>::cloneTo(
	    This& newTree, const sharedClique& subtree, const sharedClique& parent) const {
    sharedClique newClique(subtree->clone());
    newTree.addClique(newClique, parent);
    BOOST_FOREACH(const sharedClique& childClique, subtree->children()) {
      cloneTo(newTree, childClique, newClique);
    }
  }

}
/// namespace gtsam
