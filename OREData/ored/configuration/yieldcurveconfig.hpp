/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
 All rights reserved.

 This file is part of ORE, a free-software/open-source library
 for transparent pricing and risk analysis - http://opensourcerisk.org

 ORE is free software: you can redistribute it and/or modify it
 under the terms of the Modified BSD License.  You should have received a
 copy of the license along with this program.
 The license is also available online at <http://opensourcerisk.org>

 This program is distributed on the basis that it will form a useful
 contribution to risk analytics and model standardisation, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

/*! \file ored/configuration/yieldcurveconfig.hpp
    \brief Yield curve configuration classes
    \ingroup configuration
*/

#pragma once

#include <set>
#include <map>
#include <boost/optional.hpp>
#include <boost/none.hpp>
#include <boost/shared_ptr.hpp>
#include <ql/patterns/visitor.hpp>
#include <ql/types.hpp>
#include <ored/utilities/xmlutils.hpp>

using std::string;
using std::vector;
using std::set;
using std::pair;
using std::map;
using boost::optional;
using ore::data::XMLSerializable;
using ore::data::XMLNode;
using ore::data::XMLDocument;
using QuantLib::AcyclicVisitor;
using QuantLib::Real;

namespace ore {
namespace data {

//! Base class for yield curve segments.
/*!
  \ingroup configuration
*/
class YieldCurveSegment : public XMLSerializable {
public:
    //! supported segment types
    enum class Type {
        Zero,
        ZeroSpread,
        Discount,
        Deposit,
        FRA,
        Future,
        OIS,
        Swap,
        AverageOIS,
        TenorBasis,
        TenorBasisTwo,
        FXForward,
        CrossCcyBasis
    };
    //! Default destructor
    virtual ~YieldCurveSegment() {}

    //! \name Serialisation
    //@{
    virtual void fromXML(XMLNode* node) = 0;
    virtual XMLNode* toXML(XMLDocument& doc) = 0;
    //@}

    //! \name Inspectors
    //@{
    Type type() const { return type_; }
    // TODO: why typeID?
    const string& typeID() const { return typeID_; }
    const string& conventionsID() const { return conventionsID_; }
    //@}

    //! \name Visitability
    //@{
    virtual void accept(AcyclicVisitor&);
    //@}

protected:
    //! \name Constructors
    //@{
    //! Default constructor
    YieldCurveSegment() {}
    //! Detailed constructor
    YieldCurveSegment(const string& typeID, const string& conventionsID);
    //@}

private:
    // TODO: why type and typeID?
    Type type_;
    string typeID_;
    string conventionsID_;
};

//! Direct yield curve segment
/*!
  A direct yield curve segment is used when the segments is entirely defined by
  a set of quotes that are passed to the constructor.

  \ingroup configuration
*/
class DirectYieldCurveSegment : public YieldCurveSegment {
public:
    //! \name Constructors/Destructors
    //@{
    //! Default constructor
    DirectYieldCurveSegment() {}
    //! Detailed constructor
    DirectYieldCurveSegment(const string& typeID, const string& conventionsID, const vector<string>& quotes);
    //! Default destructor
    virtual ~DirectYieldCurveSegment() {}
    //@}

    //! \name Serialisation
    //@{
    virtual void fromXML(XMLNode* node);
    virtual XMLNode* toXML(XMLDocument& doc);
    //@}

    //! \name Inspectors
    //@{
    const vector<string>& quotes() const { return quotes_; }
    //@}

    //! \name Visitability
    //@{
    virtual void accept(AcyclicVisitor&);
    //@}

private:
    vector<string> quotes_;
};

//! Simple yield curve segment
/*!
  A simple yield curve segment is used when the curve segment is determined by
  a set of quotes and a projection curve.

  \ingroup configuration
*/
class SimpleYieldCurveSegment : public YieldCurveSegment {
public:
    //! \name Constructors/Destructors
    //@{
    //! Default constructor
    SimpleYieldCurveSegment() {}
    //! Detailed constructor
    SimpleYieldCurveSegment(const string& typeID, const string& conventionsID, const vector<string>& quotes,
                            const string& projectionCurveID = string());
    //! Default destructor
    virtual ~SimpleYieldCurveSegment() {}
    //@}

    //! \name Serialisation
    //@{
    virtual void fromXML(XMLNode* node);
    virtual XMLNode* toXML(XMLDocument& doc);
    //@}

    //! \name Inspectors
    //@{
    const vector<string>& quotes() const { return quotes_; }
    const string& projectionCurveID() const { return projectionCurveID_; }
    //@}

    //! \name Visitability
    //@{
    virtual void accept(AcyclicVisitor&);
    //@}

private:
    vector<string> quotes_;
    string projectionCurveID_;
};

//! Avergae OIS yield curve segment
/*!
  The average OIS yield curve segment is used e.g. for USD OIS curve building where
  the curve segment is determined by  a set of composite quotes and a projection curve.
  The composite quote is  represented here as a pair of quote strings, a tenor basis spread
  and an interest rate swap quote.

  \ingroup configuration
*/
class AverageOISYieldCurveSegment : public YieldCurveSegment {
public:
    //! \name Constructors/Destructors
    //@{
    //! Default constructor
    AverageOISYieldCurveSegment() {}
    //! Detailec constructor
    AverageOISYieldCurveSegment(const string& typeID, const string& conventionsID,
                                const vector<pair<string, string>>& quotes, const string& projectionCurveID);
    //! Default destructor
    virtual ~AverageOISYieldCurveSegment() {}
    //@}

    //! \name Serialisation
    //@{
    virtual void fromXML(XMLNode* node);
    virtual XMLNode* toXML(XMLDocument& doc);
    //@}

    //! \name Inspectors
    //@{
    const vector<pair<string, string>>& quotes() const { return quotes_; }
    const string& projectionCurveID() const { return projectionCurveID_; }
    //@}

    //! \name Visitability
    //@{
    virtual void accept(AcyclicVisitor&);
    //@}

private:
    vector<pair<string, string>> quotes_;
    string projectionCurveID_;
};

//! Tenor Basis yield curve segment
/*!
  Yield curve building from tenor basis swap quotes requires a set of tenor
  basis spread quotes and the projection curve for either the shorter or the longer tenor
  which acts as the reference curve.

  \ingroup configuration
*/
class TenorBasisYieldCurveSegment : public YieldCurveSegment {
public:
    //! \name Constructors/Destructors
    //@{
    //! Default constructor
    TenorBasisYieldCurveSegment() {}
    //! Detailed constructor
    TenorBasisYieldCurveSegment(const string& typeID, const string& conventionsID, const vector<string>& quotes,
                                const string& shortProjectionCurveID, const string& longProjectionCurveID);
    //! Default destructor
    virtual ~TenorBasisYieldCurveSegment() {}
    //@}

    //!\name Serialisation
    //@{
    virtual void fromXML(XMLNode* node);
    virtual XMLNode* toXML(XMLDocument& doc);
    //@}

    //! \name Inspectors
    //@{
    const vector<string>& quotes() const { return quotes_; }
    const string& shortProjectionCurveID() const { return shortProjectionCurveID_; }
    const string& longProjectionCurveID() const { return longProjectionCurveID_; }
    //@}

    //! \name Visitability
    //@{
    virtual void accept(AcyclicVisitor&);
    //@}

private:
    vector<string> quotes_;
    string shortProjectionCurveID_;
    string longProjectionCurveID_;
};

//! Cross Currency yield curve segment
/*!
  Cross currency basis spread adjusted discount curves for 'domestic' currency cash flows
  are built using this segment type which requires cross currency basis spreads quotes,
  the spot FX quote ID and at least the 'foreign' discount curve ID.
  Projection curves for both currencies can be provided as well for consistency with
  tenor basis in each currency.

  \ingroup configuration
*/
class CrossCcyYieldCurveSegment : public YieldCurveSegment {
public:
    //! \name Constructors/Destructors
    //@{
    //! Default constructor
    CrossCcyYieldCurveSegment() {}
    //! Detailed constructor
    CrossCcyYieldCurveSegment(const string& typeID, const string& conventionsID, const vector<string>& quotes,
                              const string& spotRateID, const string& foreignDiscountCurveID,
                              const string& domesticProjectionCurveID = string(),
                              const string& foreignProjectionCurveID = string());
    //! Default destructor
    virtual ~CrossCcyYieldCurveSegment() {}
    //@}

    //! \name Serialisation
    //@{
    virtual void fromXML(XMLNode* node);
    virtual XMLNode* toXML(XMLDocument& doc);
    //@}

    //! \name Inspectors
    //@{
    const vector<string>& quotes() const { return quotes_; }
    const string& spotRateID() const { return spotRateID_; }
    const string& foreignDiscountCurveID() const { return foreignDiscountCurveID_; }
    const string& domesticProjectionCurveID() const { return domesticProjectionCurveID_; }
    const string& foreignProjectionCurveID() const { return foreignProjectionCurveID_; }
    //@}

    //! \name Visitability
    //@{
    virtual void accept(AcyclicVisitor&);
    //@}

private:
    vector<string> quotes_;
    string spotRateID_;
    string foreignDiscountCurveID_;
    string domesticProjectionCurveID_;
    string foreignProjectionCurveID_;
};

//! Zero Spreaded yield curve segment
/*!
  A zero spreaded segment is used to build a yield curve from zero spread quotes and
  a reference yield curve.

  \ingroup configuration
*/
class ZeroSpreadedYieldCurveSegment : public YieldCurveSegment {
public:
    //! \name COnstructors/Destructors
    //@{
    //! Default constructor
    ZeroSpreadedYieldCurveSegment() {}
    //! Detailed constructor
    ZeroSpreadedYieldCurveSegment(const string& typeID, const string& conventionsID, const vector<string>& quotes,
                                  const string& referenceCurveID);
    //! Default destructor
    virtual ~ZeroSpreadedYieldCurveSegment() {}
    //@}

    //! \name Serialisation
    //@{
    virtual void fromXML(XMLNode* node);
    virtual XMLNode* toXML(XMLDocument& doc);
    //@}

    //! \name Inspectors
    //@{
    const vector<string>& quotes() const { return quotes_; }
    const string& referenceCurveID() const { return referenceCurveID_; }
    //@}

    //! \name Visitability
    //@{
    virtual void accept(AcyclicVisitor&);
    //@}

private:
    vector<string> quotes_;
    string referenceCurveID_;
};

//! Yield Curve configuration
/*!
  Wrapper class containing all yield curve segments needed to build a yield curve.

  \ingroup configuration
 */
class YieldCurveConfig : public XMLSerializable {
public:
    //! \name Constructors/Destructurs
    //@{
    //! Default constructor
    YieldCurveConfig() {}
    //! Detailed constructor
    YieldCurveConfig(const string& curveID, const string& curveDescription, const string& currency,
                     const string& discountCurveID, const vector<boost::shared_ptr<YieldCurveSegment>>& curveSegments,
                     const string& interpolationVariable = "Discount", const string& interpolationMethod = "LogLinear",
                     const string& zeroDayCounter = "A365", bool extrapolation = true, Real tolerance = 1.0e-12);
    //! Default destructor
    virtual ~YieldCurveConfig() {}
    //@}

    //! \name Serilalisation
    //@{
    virtual void fromXML(XMLNode* node);
    virtual XMLNode* toXML(XMLDocument& doc);
    //@}

    //! \name Inspectors
    //@{
    const string& curveID() const { return curveID_; }
    const string& curveDescription() const { return curveDescription_; }
    const string& currency() const { return currency_; }
    const string& discountCurveID() const { return discountCurveID_; }
    const vector<boost::shared_ptr<YieldCurveSegment>>& curveSegments() const { return curveSegments_; }
    const string& interpolationVariable() const { return interpolationVariable_; }
    const string& interpolationMethod() const { return interpolationMethod_; }
    const string& zeroDayCounter() const { return zeroDayCounter_; }
    bool extrapolation() const { return extrapolation_; }
    Real tolerance() const { return tolerance_; }
    const set<string>& requiredYieldCurveIDs() const { return requiredYieldCurveIDs_; }
    //@}

    //! \name Setters
    //@{
    string& interpolationVariable() { return interpolationVariable_; }
    string& interpolationMethod() { return interpolationMethod_; }
    string& zeroDayCounter() { return zeroDayCounter_; }
    bool& extrapolation() { return extrapolation_; }
    Real& tolerance() { return tolerance_; }
    //@}

private:
    void populateRequiredYieldCurveIDs();

    // Mandatory members
    string curveID_;
    string curveDescription_;
    string currency_;
    string discountCurveID_;
    vector<boost::shared_ptr<YieldCurveSegment>> curveSegments_;
    set<string> requiredYieldCurveIDs_;

    // Optional members
    string interpolationVariable_;
    string interpolationMethod_;
    string zeroDayCounter_;
    bool extrapolation_;
    Real tolerance_;
};

// Map form curveID to YieldCurveConfig
using YieldCurveConfigMap = std::map<string, boost::shared_ptr<YieldCurveConfig>>;
}
}