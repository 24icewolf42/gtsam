/**
 * @file    VectorConfig.h
 * @brief   Factor Graph Configuration
 * @author  Carlos Nieto
 * @author  Christian Potthast
 */

// \callgraph

#pragma once

#include <map>
#include <boost/serialization/map.hpp>

#include "Testable.h"
#include "Vector.h"

namespace gtsam {
	
  /** Factor Graph Configuration */
  class VectorConfig : public Testable<VectorConfig> {

  protected:
    /** Map from string indices to values */
    std::map<std::string, Vector> values;

  public:
    typedef std::map<std::string, Vector>::iterator iterator;
    typedef std::map<std::string, Vector>::const_iterator const_iterator;

    VectorConfig() {}
    VectorConfig(const VectorConfig& cfg_in): values(cfg_in.values) {}
    
    virtual ~VectorConfig() {}

    /** return all the nodes in the graph **/
    std::vector<std::string> get_names() const;

    /** Insert a value into the configuration with a given index */
    VectorConfig& insert(const std::string& name, const Vector& val);

    /** Add to vector at position j */
    void add(const std::string& j, const Vector& a);

    /**
     * Add a delta config, needed for use in NonlinearOptimizer
     * For VectorConfig, this is just addition.
     */
    VectorConfig exmap(const VectorConfig& delta) const;

    /**
     * Add a delta vector (not a config)
     * Will use the ordering that map uses to loop over vectors
     */
    VectorConfig exmap(const Vector& delta) const;

    const_iterator begin() const {return values.begin();}
    const_iterator end()   const {return values.end();}

    /** get a vector in the configuration by name */
    const Vector& get(const std::string& name) const;

    /** operator[] syntax for get */
    inline const Vector& operator[](const std::string& name) const { return get(name); }

    bool contains(const std::string& name) const {
      const_iterator it = values.find(name);
      return (it!=values.end());
    }

    /** Nr of vectors */
    size_t size() const { return values.size();}

    /** Total dimensionality */
    size_t dim() const;

    /** Scale */
    VectorConfig scale(double s) const;
    VectorConfig operator*(double s) const;

    /** Add */
    VectorConfig operator+(const VectorConfig &b) const;

    /** Subtract */
    VectorConfig operator-(const VectorConfig &b) const;

    /** print */
    void print(const std::string& name = "") const;

    /** equals, for unit testing */
    bool equals(const VectorConfig& expected, double tol=1e-9) const;

    void clear() {values.clear();}
    
    /** Dot product */
    double dot(const VectorConfig& b) const;
    
  private:
    /** Serialization function */
    friend class boost::serialization::access;
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
    {
      ar & BOOST_SERIALIZATION_NVP(values);
    }
  }; // VectorConfig

  /** Dot product */
  double dot(const VectorConfig&, const VectorConfig&);

} // gtsam
