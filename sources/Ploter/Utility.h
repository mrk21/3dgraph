/**
 * 
 * @file   Utility.h
 * @author 村田 裕一
 * @brief  。
 * 
 *///---------------------------------------------------------------------------
#ifndef     INCLUDED_UTILITY_H
#define     INCLUDED_UTILITY_H


#include    <string>
#include    <vector>
#include    <algorithm>
#include    <cmath>
#include    <boost/function.hpp>
#include    <boost/lambda/lambda.hpp>
#include    <boost/lambda/bind.hpp>
#include    <boost/lambda/if.hpp>
#include    <boost/numeric/ublas/matrix.hpp>
#include    <boost/numeric/ublas/vector.hpp>
#include    <boost/numeric/ublas/matrix_proxy.hpp>
#include    <boost/numeric/ublas/vector_proxy.hpp>
#include    <boost/numeric/ublas/triangular.hpp>
#include    <boost/numeric/ublas/lu.hpp>
#include    <boost/numeric/ublas/io.hpp>
#include    <windows.h>
#include    <gl/gl.h>


namespace Ploter {
    
    using namespace boost;
    using namespace boost::numeric;
    using namespace boost::numeric::ublas;
    using namespace boost::lambda;
    
    
    typedef double                 Element;
    typedef ublas::vector<Element> Vector;
    typedef ublas::matrix<Element> Matrix;
    
    typedef unsigned int           Index;
    typedef std  ::vector<Vector>  VertexList;
    typedef ublas::matrix<Vector>  VertexMatrix;
    
    typedef function<      Element& (      Vector&)> ReferenceFunction;
    typedef function<const Element& (const Vector&)> ConstReferenceFunction;
    
    
    const Element  ELEMENT_EXTREAM_VALUE = 1.79769313486232e308;
    
    
    /**
     * @brief ベクトルの要素の指標を表す値。
     *///---------------------------------------------------------------------------
    enum { X,Y,Z,W };
    
    
    /**
     * @brief ベクトル・行列に関する定数郡。
     *///---------------------------------------------------------------------------
    namespace V {
        
        const Vector  ZERO  ( zero_vector<Element>(4)   );
        const Vector  X_UNIT( unit_vector<Element>(4,X) );
        const Vector  Y_UNIT( unit_vector<Element>(4,Y) );
        const Vector  Z_UNIT( unit_vector<Element>(4,Z) );
        const Vector  W_UNIT( unit_vector<Element>(4,W) );
        
        const      ReferenceFunction        X( ret<      Element&>( _1[0] ) );
        const      ReferenceFunction        Y( ret<      Element&>( _1[1] ) );
        const      ReferenceFunction        Z( ret<      Element&>( _1[2] ) );
        const ConstReferenceFunction  CONST_X( ret<const Element&>( _1[0] ) );
        const ConstReferenceFunction  CONST_Y( ret<const Element&>( _1[1] ) );
        const ConstReferenceFunction  CONST_Z( ret<const Element&>( _1[2] ) );
    }
    namespace M {
        
        const Matrix  ZERO    ( zero_matrix    <Element>(4,4) );
        const Matrix  IDENTITY( identity_matrix<Element>(4,4) );
    }
    
    
    /**
     * @brief 度からラジアンへの変換。 
     *///---------------------------------------------------------------------------
    inline double  toRad( double deg ) {
        
        return deg/180 * M_PI;
    }
    
    
    /**
     * @brief 二重のfor_each。
     *///---------------------------------------------------------------------------
    template<class IteratorType, class UnaryFunctionType>
    void  dualForEach( IteratorType begin,  IteratorType end,  UnaryFunctionType  unaryFunction ) {
        
        for ( IteratorType  i = begin;  i != end;  i++ ) {
            
            std::for_each( i.begin(), i.end(), unaryFunction );
        }
    }
    
    /**
     * @brief 対数縮尺にする。ただし、0は無視する。
     *///---------------------------------------------------------------------------
    void  toLogScale( VertexMatrix& object,  ReferenceFunction element ) {
        
        dualForEach( object.begin1(), object.end1(),
            
            if_( bind(V::Z,_1) != 0 ) [
                
                bind(element,_1) = bind<double,double>( std::log10, bind(element,_1) ) + 1
            ]
        );
    }
    
    
    /**
     * @brief 正規化する。
     *///---------------------------------------------------------------------------
    template<class IteratorType>
    void  regularize( IteratorType begin,  IteratorType end ) {
        
        Element  sumZ;
        
        for ( IteratorType  i = begin;  i != end;  i++ ) {
            
            sumZ = 0;
            
            std::for_each( i.begin(), i.end(),
                
                var(sumZ) += bind<double,double>( std::fabs, bind(V::Z,_1) )
            );
            
            if ( sumZ != 0 ) {
                
                std::for_each( i.begin(), i.end(), ( bind(V::Z,_1) /= sumZ ) );
            }
        }
    }
    
    
    /**
     * @brief 頂点vertexを描画する。
     *///---------------------------------------------------------------------------
    inline void  drawVertex( const Vector& vertex ) {
        
        glVertex4d( vertex(X), vertex(Y), vertex(Z), vertex(W) );
    }
    
    
    /**
     * @brief 始点をorigin、終点をorigin + toとする線分を行列affineで座標変換した後、描画する。
     *///---------------------------------------------------------------------------
    void  drawLine( const Matrix& affine,  const Vector& origin,  const Vector& to ) {
        
        static Vector  begin( V::ZERO );
        static Vector  end  ( V::ZERO );
        
        begin = prod( affine, origin +      V::W_UNIT );
        end   = prod( affine, origin + to + V::W_UNIT );
        
        drawVertex( begin );
        drawVertex( end   );
    }
    
    
    /**
     * @brief 方向がdirectionで始点の集合がpositionsな平行線の集合を行列affineで座標変換した後、描画する。
     *///---------------------------------------------------------------------------
    void  drawParallelLines( const Matrix& affine,  const VertexList& positions,
                                                    const Vector&     direction ) {
        
        glBegin( GL_LINES ); {
            
            std::for_each( positions.begin(), positions.end(), bind( drawLine, affine, _1, direction ) );
            
        } glEnd();
    }
    
    
    /**
     * @brief ベクトルv(x,y,z)の生成。
     *///---------------------------------------------------------------------------
    inline Vector  v( Element x,  Element y,  Element z ) {
        
        Vector  v( V::ZERO );
        
        v(X) = x;
        v(Y) = y;
        v(Z) = z;
        
        return v;
    }
    
    
    /**
     * @brief ベクトルtargetを行列affineで座標変換する。
     *///---------------------------------------------------------------------------
    inline void  transform( const Matrix& affine,  Vector& target ) {
        
        target = prod( affine, target + V::W_UNIT );
    }
    
    
    /**
     * @brief x軸,y軸,z軸をそれぞれ、a倍,b倍,c倍する行列を返す。
     *///---------------------------------------------------------------------------
    inline Matrix  scale( Element a,  Element b,  Element c ) {
        
        Matrix  m( M::IDENTITY );
        
        m(0,0) = a;
        m(1,1) = b;
        m(2,2) = c;
        
        return m;
    }
    
    
    /**
     * @brief toだけ平行移動する行列を返す。
     *///---------------------------------------------------------------------------
    inline Matrix  translate( const Vector &to ) {
        
        Matrix  m( M::IDENTITY );
        
        m(0,3) = to(X);
        m(1,3) = to(Y);
        m(2,3) = to(Z);
        
        return m;
    }
    
    
    /**
     * @brief 軸axis を deg°回転する行列を返す。
     *///---------------------------------------------------------------------------
    Matrix  rotate( Element deg,  const Vector& axis ) {
        
        Matrix  m( M::IDENTITY );
        
        Element  sinValue = std::sin( toRad(deg) );
        Element  cosValue = std::cos( toRad(deg) );
        
        for ( int i=X; i<=Z; i++ ) {
            
            for ( int j=X; j<=Z; j++ ) {
                
                m(i,j) = axis(i) * axis(j) * (1-cosValue);
            }
            
            m(i,i) += cosValue;
        }
        
        m(0,1) -= axis(Z) * sinValue,  m(0,2) += axis(Y) * sinValue;
        m(1,0) += axis(Z) * sinValue,  m(1,2) -= axis(X) * sinValue;
        m(2,0) -= axis(Y) * sinValue,  m(2,1) += axis(X) * sinValue;
        
        return m;
    }
    
    
    /**
     * @brief 軸axis を direction の方向に deg°せん断変形する行列を返す。
     *///---------------------------------------------------------------------------
    Matrix  shear( Element deg,  const Vector& axis,  const Vector& direction ) {
        
        Matrix  m( M::IDENTITY );
        
        Element  sinValue = std::sin( toRad(deg) );
        
        for ( int i=X; i<=Z; i++ ) {
            
            for ( int j=X; j<=Z; j++ ) {
                
                if (i!=j) {
                    
                    m(i,j) = direction(i) * axis(j) * sinValue;
                }
            }
        }
        
        return m;
    }
    
    
    /**
     * @brief 連立方程式 Ax=b を解く。
     *///---------------------------------------------------------------------------
    Vector& solve( Matrix& A,  Vector& b ) {
        
        permutation_matrix<>  pm( A.size1() );
        
        lu_factorize (A,pm);
        lu_substitute(A,pm,b);
        
        return b;
    }
    
    
    /**
     *
     * @brief オプションを管理するクラス。
     *
     *///---------------------------------------------------------------------------
    class Option {
        
      public:
        
        Option( const std::string& section_ ) : section( section_ ) {}
        
        
        /**
         * @brief iniファイルからkeyが指す値を読み込む。
         *///---------------------------------------------------------------------------
        int  get( const std::string& key ) const {
            
            return  GetPrivateProfileInt( this->section.c_str(), key.c_str(), 1, PATH.c_str() );
        }
        
      private:
        
        /**
         * @brief iniファイルのパスを設定する。
         *///---------------------------------------------------------------------------
        static std::string  initialize( void ) {
            
            char  path[64];
            
            GetCurrentDirectory( 64, path );
            
            return  std::string(path) + "\\option.ini";
        }
        
        static const std::string  PATH;
                     std::string  section;
    };
    
    const std::string  Option::PATH( Option::initialize() );
    
    
    /**
     *
     * @brief 最大値と最小値の格納。
     *
     *///---------------------------------------------------------------------------
    class MinMax {
        
      public:
        
        enum { INNER, UPPER, LOWER };
        
        /**
         * @brief 最小値・最大値の初期化。
         *///---------------------------------------------------------------------------
        void  initialize( void ) {
            
            this->min =  ELEMENT_EXTREAM_VALUE;
            this->max = -ELEMENT_EXTREAM_VALUE;
        }
        
        
        /**
         * @brief アクセッサ。
         *///---------------------------------------------------------------------------
        Element  getMin( void ) const { return this->min; }
        Element  getMax( void ) const { return this->max; }
        
        
        /**
         * @brief valueが最小値・最大値かどうかを査定し、更新する。
         *///---------------------------------------------------------------------------
        int  assess( Element value ) {
            
            int  state = MinMax::INNER;
            
            if ( this->min > value ) { this->min = value;  state = MinMax::UPPER; }
            if ( this->max < value ) { this->max = value;  state = MinMax::LOWER; }
            
            return  state;
        }
        
        
        /**
         * @brief 最小値・最大値の範囲の取得。
         *///---------------------------------------------------------------------------
        Element  getRange( void ) const {
            
            return  this->max - this->min;
        }
        
        
        /**
         * @brief 最小値・最大値の中間値の取得。
         *///---------------------------------------------------------------------------
        Element  getMiddlePoint( void ) const {
            
            return  ( this->max + this->min ) / 2;
        }
        
      private:
        
        Element  min;
        Element  max;
    };
    
    
    /**
     *
     * @brief x,y,zそれぞれの最小値・最大値を管理するクラス。
     *
     *///---------------------------------------------------------------------------
    class MinMaxManager {
        
      public:
        
        /**
         * @brief objectのx,y,zそれぞれの最小値・最大値を設定する。
         *///---------------------------------------------------------------------------
        void  set( const VertexMatrix& object ) {
            
            this->x.initialize();
            this->y.initialize();
            this->z.initialize();
            
            dualForEach( object.begin1(), object.end1(), (
                
                bind( &MinMax::assess, &this->x, bind(V::CONST_X,_1) ),
                bind( &MinMax::assess, &this->y, bind(V::CONST_Y,_1) ),
                bind( &MinMax::assess, &this->z, bind(V::CONST_Z,_1) )
            ));
        }
        
        
        MinMax  x;
        MinMax  y;
        MinMax  z;
    };
    
    
    /**
     *
     * @brief アフィン行列を管理するクラス。 
     *
     *///---------------------------------------------------------------------------
    class AffineManager {
        
      public:
        
        enum {
            
            FOR_X_AXIS,
            FOR_Y_AXIS,
            FOR_Z_AXIS,
            FOR_XY_GRID,
            FOR_XZ_GRID,
            FOR_YZ_GRID,
            FOR_GRAPH,
        };
        
        
        AffineManager( const Option& option_,  const MinMaxManager*  minmaxManager_) :
            
            option( option_),  minmaxManager( minmaxManager_)
        {}
        
        
        /**
         * @brief 各グラフ要素用のアフィン行列を返す。
         *///---------------------------------------------------------------------------
        Matrix  get( int target ) const {
            
            switch( target ) {
                
              case FOR_X_AXIS:   return  this->forXAxis();
              case FOR_Y_AXIS:   return  this->forYAxis();
              case FOR_Z_AXIS:   return  this->forZAxis();
              case FOR_XZ_GRID:  return  this->forXzGrid();
              case FOR_GRAPH:    return  this->forGraph();
              default:           return  this->common();
                
            }
        }
        
      private:
        
        /**
         * @brief 共通のアフィン行列。
         *///---------------------------------------------------------------------------
        Matrix  common( void ) const {
            
            return  prod( this->projection(),
                
                prod<Matrix>( this->horizontal(), this->shift() )
            );
        }
        
        
        /**
         * @brief x軸用のアフィン行列。
         *///---------------------------------------------------------------------------
        Matrix  forXAxis( void ) const {
            
            return  prod( this->common(),
                
                this->selectMatrix(
                    
                    M::IDENTITY,
                    prod( translate( V::Y_UNIT ), scale( 1,-1,1 ) )
                )
            );
        }
        
        
        /**
         * @brief y軸用のアフィン行列。
         *///---------------------------------------------------------------------------
        Matrix  forYAxis( void ) const {
            
            return  prod( this->common(), translate( V::X_UNIT ) );
        }
        
        
        /**
         * @brief z軸用のアフィン行列。
         *///---------------------------------------------------------------------------
        Matrix  forZAxis( void ) const {
            
            return  prod( this->common(),
                
                this->selectMatrix(
                    
                    M::IDENTITY,
                    prod( translate( V::X_UNIT ), rotate( 90, V::Z_UNIT ) )
                )
            );
        }
        
        
        /**
         * @brief xyグリッド用のアフィン行列。
         *///---------------------------------------------------------------------------
        Matrix  forXzGrid( void ) const {
            
            return  prod( this->common(),
                
                this->selectMatrix( translate( V::Y_UNIT ), M::IDENTITY )
            );
        }
        
        
        /**
         * @brief グラフ用のアフィン行列。
         *///---------------------------------------------------------------------------
        Matrix  forGraph( void ) const {
            
            return  prod( this->common(),
                
                prod<Matrix>( this->regularize(), this->toOrigin() )
            );
        }
        
        
        /**
         * @brief 投影行列を返す。
         *///---------------------------------------------------------------------------
        Matrix  projection( void ) const {
            
            Matrix  m( M::IDENTITY );
            
            m(2,2) = 0;
            m(0,2) = -std::cos( toRad(45) );
            m(1,2) = -std::sin( toRad(45) );
            
            return  m;
        }
        
        
        /**
         * @brief 軸を水平にする。
         *///---------------------------------------------------------------------------
        Matrix  horizontal( void ) const {
            
            return  prod( rotate( -90, V::X_UNIT ),
                
                selectMatrix( M::IDENTITY, rotate( -90, V::Z_UNIT ) )
            );
        }
        
        
        /**
         * @brief x軸の移動。
         *///---------------------------------------------------------------------------
        Matrix  shift( void ) const {
            
            return  selectMatrix( M::IDENTITY, translate( -V::X_UNIT ) );
        }
        
        
        /**
         * @brief 原点に移動。
         *///---------------------------------------------------------------------------
        Matrix  toOrigin( void ) const {
            
            return  translate(
                
                v( -this->minmaxManager->x.getMin(),
                   -this->minmaxManager->y.getMin(),
                   -this->minmaxManager->z.getMin() )
            );
        }
        
        
        /**
         * @brief スケールを正規化。
         *///---------------------------------------------------------------------------
        Matrix  regularize( void ) const {
            
            return  scale(
                
                1.0 / this->getScaleRange( "x", this->minmaxManager->x ),
                1.0 / this->getScaleRange( "y", this->minmaxManager->y ),
                0.5 / this->getScaleRange( "z", this->minmaxManager->z )
            );
        }
        
        
        /**
         * @brief スケールを取得。
         *///---------------------------------------------------------------------------
        Element  getScaleRange( const std::string& key,  const MinMax& minmax ) const {
            
            std::string  section( key + "Axis" );
            
            return  Option(section).get("isFix") ?
                
                minmax.getRange() : Option(section).get("maxValue");
        }
        
        
        /**
         * @brief 水平軸の違いによるアフィン行列の選択。
         *///---------------------------------------------------------------------------
        const Matrix&  selectMatrix( const Matrix& caseX,  const Matrix& caseY ) const {
            
            return  this->option.get("horizontalAxis") ? caseY : caseX;
        }
        
        
        Option                option;
        const MinMaxManager*  minmaxManager;
    };
}
#endif
